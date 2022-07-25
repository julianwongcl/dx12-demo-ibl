#include "DescHeapSRVCommon.h"
#include "ExceptionHelper.h"

DescHeapSRVCommon::DescHeapSRVCommon(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // Describe and create a shader resource view (SRV) heap for the two textures
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 2;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&descHeapSrvCommon)));
}
