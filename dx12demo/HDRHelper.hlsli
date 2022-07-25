#define PI 3.1415926538

float2 dirToRect(float3 v)
{
    const float x = 0.5f - atan2(v.x, v.z) / (2.0 * PI);
    const float y = acos(v.y) / PI;
    return float2(x, y);
}

float4 linearToSRGB(float4 color)
{
    const float brightness = 1.0f;
    const float contrast = 0.7f;
    const float k_srgb = 2.2f;
    color.rgb = brightness * pow(abs(color.xyz), float3(contrast, contrast, contrast));
    color.rgb = pow(color.xyz, float3(k_srgb, k_srgb, k_srgb));
    return color;
}
