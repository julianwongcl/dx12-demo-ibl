#pragma pack_matrix( row_major )

#include "HDRHelper.hlsli"

#define PI 3.1415926538

cbuffer cbPerObject : register(b0)
{
	float4x4 mtx_proj;
	float4x4 mtx_view;
	float4x4 mtx_world;
	float4x4 mtx_viewinverse;
	float seq_hammersley[64];
};

struct PSInput
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
	float3 position_camera : POSITION0;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
	const float3 v_sky = -normalize(input.position_camera);
	const float2 access = dirToRect(v_sky);
	const float4 color = g_texture.SampleLevel(g_sampler, access, 0);
	return linearToSRGB(color);
}
