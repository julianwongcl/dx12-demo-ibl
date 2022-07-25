#include "HDRHelper.hlsli"
#include "Structures.hlsli"

#define PI 3.1415926538

cbuffer cbPerObject : register(b0)
{
    float4x4 mtx_proj;
    float4x4 mtx_view;
    float4x4 mtx_world;
    float4x4 mtx_viewinverse;
    float hammersley[128];
};

Texture2D tex_hdr : register(t0);
Texture2D tex_ir  : register(t1);
SamplerState sampler_hdr : register(s0);

float clamped_dot(float floor, float3 v1, float3 v2)
{
    return max(floor, dot(v1, v2));
}

float3 SchlickApprox(float cosT, float3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosT, 5.0);
}

float3x3 rotation(float3 r)
{
    const float3 A = normalize(cross(float3(0.0, 0.0, 1.0), r));
    const float3 B = normalize(cross(r, A));
    return float3x3(A, B, r);
}

float4 main(PSInPositionNormalTexture input) : SV_TARGET
{
    const uint kSamples = 64;

    const float roughness = 10.0;
    const float invs = 1.0 / (roughness + 1.0);
    const float3 teapot_diffuse = { 1.0, 1.0, 1.0 };
    const float3 teapot_specular = { 1.0, 1.0, 1.0 };

    const float3 V = normalize(input.position_camera).xyz;
    const float3 N = normalize(input.normal);
    const float3 R = reflect(-V, N);

    const float3 color_ir = tex_ir.SampleLevel(sampler_hdr, dirToRect(N), 0).xyz;
    const float3 color_diffuse = color_ir * teapot_diffuse / PI;

    float texDimsW = 0.0;
    float texDimsH = 0.0;
    tex_hdr.GetDimensions(texDimsW, texDimsH);
    const float log2area = 0.5 * log2((texDimsW * texDimsH) / float(kSamples));

    // accumulate 64 samples
    float3 color_specular = { 0.0, 0.0, 0.0 };
    for (uint i = 0; i < kSamples; ++i)
    {
        const float u = acos(pow(abs(hammersley[i * 2]), invs)) / PI;
        const float v = hammersley[i * 2 + 1];
        const float3 hdr_dir = { cos(2.0 * PI * (0.5 - u)) * sin(PI * v),
                                 sin(2.0 * PI * (0.5 - u)) * sin(PI * v),
                                 cos(PI * v) };
        const float3 rotated_dir = mul(hdr_dir, rotation(R));
        const float3 L = normalize(rotated_dir);
        const float2 hdr_uv = dirToRect(L);

        // distribution
        const float3 H = normalize(V + L);
        const float D = (roughness + 2) / ((2.0 * PI) * pow(clamped_dot(0.0, N, H), roughness));
        const float level = log2area - 0.5 * log2(D);
        const float3 color_hdr = tex_hdr.SampleLevel(sampler_hdr, hdr_uv, level).xyz;

        // fresnel
        const float NdotL = clamped_dot(0.0, N, L);
        const float LdotH = clamped_dot(0.0, L, H);
        const float3 F = SchlickApprox(LdotH, teapot_specular);

        // shadowing
        const float G = 1.0 / pow(LdotH, 2.0);

        // brdf
        color_specular += F * G * color_hdr * NdotL;
    }
    color_specular /= float(kSamples);

    float4 color = { 1.0, 1.0, 1.0, 1.0 };
    color.rgb = color_diffuse + color_specular;
    return linearToSRGB(color);
}
