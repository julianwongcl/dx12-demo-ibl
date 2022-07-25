struct VSOutPositionNormalTexture
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float4 position_camera : POSITION0;
};

struct PSInPositionNormalTexture
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float4 position_camera : POSITION0;
};
