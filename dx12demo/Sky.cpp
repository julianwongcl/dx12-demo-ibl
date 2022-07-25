#include "ExceptionHelper.h"
#include "Sky.h"
#include "Quaternion.h"
#include "RootSignatureCommon.h"
#include "DescHeapSRVCommon.h"

Sky::Sky(ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    RootSignatureCommon* rootSignature)
{
    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        // load csos
        HRESULT hr;
        hr = D3DReadFileToBlob(L"vs_sky.cso", &vertexShader);
        ThrowIfFailed(hr);
        hr = D3DReadFileToBlob(L"ps_sky.cso", &pixelShader);
        ThrowIfFailed(hr);

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = DirectX::GeometricPrimitive::VertexType::InputLayout;
        psoDesc.pRootSignature = rootSignature->rootSignatureCommon;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // don't cull the sphere
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.StencilEnable = TRUE;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    m_sphere = DirectX::GeometricPrimitive::CreateSphere(10.0f, 16, true, false, device);
    m_transform = Framework::Vqs();
}

void Sky::RecordCommandList(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(m_pipelineState.Get());
    m_sphere->Draw(commandList);
}
