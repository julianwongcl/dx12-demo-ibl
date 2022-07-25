#pragma once
#include "d3dx12.h"

class RootSignatureCommon
{
public:
    RootSignatureCommon(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    ~RootSignatureCommon();
    ID3D12RootSignature* rootSignatureCommon = nullptr;
};
