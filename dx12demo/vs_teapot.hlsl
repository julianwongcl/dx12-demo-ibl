#pragma pack_matrix( row_major )

#include "Structures.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 mtx_proj;
    float4x4 mtx_view;
    float4x4 mtx_world;
    float4x4 mtx_viewinverse;
    float seq_hammersley[128];
};

VSOutPositionNormalTexture main(float4 position : SV_Position,
                                float3 normal : NORMAL,
                                float2 uv : TEXCOORD)
{
    const float4 position_w = mul(position, mtx_world);
    const float4 position_wv = mul(position_w, mtx_view);
    const float4 position_wvp = mul(position_wv, mtx_proj);
    const float4 position_camera = mul(float4(0, 0, 0, 1), mtx_viewinverse) - position_w;

    VSOutPositionNormalTexture result;
    result.position = position_wvp;
    result.normal = normal;
    result.uv = uv;
    result.position_camera = position_camera;
    return result;
}
