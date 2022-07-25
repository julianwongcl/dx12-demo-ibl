#pragma once
#include <DirectXMath.h>
using namespace DirectX;
struct ConstBuffWVP
{
    XMMATRIX mtx_proj;
    XMMATRIX mtx_view;
    XMMATRIX mtx_world;
    XMMATRIX mtx_viewinv;
    float    seq_hammersley[128];
};
static_assert((sizeof(ConstBuffWVP) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
