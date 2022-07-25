#include "Teapot.h"

#include "DescHeapSRVCommon.h"
#include "ExceptionHelper.h"
#include "RootSignatureCommon.h"

Teapot::Teapot(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, RootSignatureCommon* rootSignature)
{
    m_teapot = DirectX::GeometricPrimitive::CreateTeapot(1.0f, 8, false, device);
    m_transform = Framework::Vqs();

    // Create the pipeline state, which includes compiling and loading shaders.
    {

        ComPtr<ID3DBlob> blobVS = nullptr;
        ComPtr<ID3DBlob> blobPS = nullptr;

        // load csos
        HRESULT hr;
        hr = D3DReadFileToBlob(L"vs_teapot.cso", &blobVS);
        ThrowIfFailed(hr);
        hr = D3DReadFileToBlob(L"ps_teapot.cso", &blobPS);
        ThrowIfFailed(hr);

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = DirectX::GeometricPrimitive::VertexType::InputLayout;
        psoDesc.pRootSignature = rootSignature->rootSignatureCommon;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(blobVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(blobPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.StencilEnable = TRUE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }
}


void Teapot::RecordCommandList(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(m_pipelineState.Get());
    m_teapot->Draw(commandList);
}
