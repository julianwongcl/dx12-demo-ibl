#include "Textures.h"
#include "ExceptionHelper.h"
#include "DescHeapSRVCommon.h"

Textures::Textures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DescHeapSRVCommon* srvHeap)
{
    HRESULT hr = DirectX::LoadFromHDRFile(L"newport.hdr", nullptr, m_scratchimage_hdr);
    ThrowIfFailed(hr);
    hr = DirectX::LoadFromHDRFile(L"newportir.hdr", nullptr, m_scratchimage_ir);
    ThrowIfFailed(hr);

    hr = DirectX::GenerateMipMaps(*m_scratchimage_hdr.GetImage(0, 0, 0),
        DirectX::TEX_FILTER_FORCE_NON_WIC,
        8,
        m_mipmaps_hdr,
        false);
    ThrowIfFailed(hr);
    hr = DirectX::GenerateMipMaps(*m_scratchimage_ir.GetImage(0, 0, 0),
        DirectX::TEX_FILTER_FORCE_NON_WIC,
        8,
        m_mipmaps_ir,
        false);
    ThrowIfFailed(hr);

    hr = DirectX::CreateTexture(device, m_mipmaps_hdr.GetMetadata(), &m_texture_hdr);
    ThrowIfFailed(hr);

    std::vector<D3D12_SUBRESOURCE_DATA> subresources_hdr;
    hr = DirectX::PrepareUpload(device,
        m_mipmaps_hdr.GetImages(),
        m_mipmaps_hdr.GetImageCount(),
        m_mipmaps_hdr.GetMetadata(),
        subresources_hdr);
    ThrowIfFailed(hr);

    const UINT64 uploadBufferSize_hdr = GetRequiredIntermediateSize(m_texture_hdr.Get(),
        0,
        static_cast<unsigned int>(subresources_hdr.size()));
    static const auto HeapPropertyUploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const auto descUploadBufferSize_hdr = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize_hdr);
    hr = device->CreateCommittedResource(
        &HeapPropertyUploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &descUploadBufferSize_hdr,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadHeap_hdr));
    ThrowIfFailed(hr);

    UpdateSubresources(commandList,
        m_texture_hdr.Get(),
        m_uploadHeap_hdr.Get(),
        0, 0,
        static_cast<unsigned int>(subresources_hdr.size()),
        subresources_hdr.data());

    hr = DirectX::CreateTexture(device, m_mipmaps_ir.GetMetadata(), &m_texture_ir);
    ThrowIfFailed(hr);

    std::vector<D3D12_SUBRESOURCE_DATA> subresources_ir;
    hr = DirectX::PrepareUpload(device,
        m_mipmaps_hdr.GetImages(),
        m_mipmaps_hdr.GetImageCount(),
        m_mipmaps_hdr.GetMetadata(),
        subresources_ir);
    ThrowIfFailed(hr);

    const UINT64 uploadBufferSize_ir = GetRequiredIntermediateSize(m_texture_ir.Get(), 0, static_cast<unsigned int>(subresources_ir.size()));
    const auto descUploadBufferSize_ir = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize_ir);
    hr = device->CreateCommittedResource(
        &HeapPropertyUploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &descUploadBufferSize_ir,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadHeap_ir));
    ThrowIfFailed(hr);

    UpdateSubresources(commandList,
        m_texture_ir.Get(),
        m_uploadHeap_ir.Get(),
        0, 0,
        static_cast<unsigned int>(subresources_ir.size()),
        subresources_ir.data());

    // transition both resource barriers
    const auto hdrFromCopyToShaderResource = CD3DX12_RESOURCE_BARRIER::Transition(m_texture_hdr.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &hdrFromCopyToShaderResource);

    const auto irFromCopyToShaderResource = CD3DX12_RESOURCE_BARRIER::Transition(m_texture_ir.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &irFromCopyToShaderResource);

    // HDR SRV descriptor
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescHDR = {};
    srvDescHDR.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescHDR.Format = m_mipmaps_hdr.GetMetadata().format;
    srvDescHDR.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescHDR.Texture2D.MipLevels = 8;

    // IR SRV descriptor
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescIR = {};
    srvDescIR.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescIR.Format = m_mipmaps_ir.GetMetadata().format;
    srvDescIR.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescIR.Texture2D.MipLevels = 8;

    // create HDR SRV
    D3D12_CPU_DESCRIPTOR_HANDLE handle_ir = srvHeap->descHeapSrvCommon->GetCPUDescriptorHandleForHeapStart();
    device->CreateShaderResourceView(m_texture_hdr.Get(), &srvDescHDR, handle_ir);

    // create IR SRV
    handle_ir.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateShaderResourceView(m_texture_ir.Get(), &srvDescIR, handle_ir);
}
