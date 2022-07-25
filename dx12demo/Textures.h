#pragma once
#include "d3dx12.h"
#include <DirectXTex.h>

class DescHeapSRVCommon;

using Microsoft::WRL::ComPtr;

class Textures
{
public:
    Textures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DescHeapSRVCommon* srvHeap);


private:
    ComPtr<ID3D12Resource> m_uploadHeap_hdr;
    ComPtr<ID3D12Resource> m_uploadHeap_ir;
    ComPtr<ID3D12Resource> m_texture_hdr;
    ComPtr<ID3D12Resource> m_texture_ir;

    DirectX::ScratchImage m_scratchimage_hdr;
    DirectX::ScratchImage m_scratchimage_ir;
    DirectX::ScratchImage m_mipmaps_hdr;
    DirectX::ScratchImage m_mipmaps_ir;
};
