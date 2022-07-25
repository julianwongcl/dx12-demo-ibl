#pragma once
#include "d3dx12.h"

class DescHeapSRVCommon
{
public:
    DescHeapSRVCommon(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    ID3D12DescriptorHeap* descHeapSrvCommon;
};
