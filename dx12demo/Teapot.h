#pragma once

#include "d3dx12.h"
#include "Quaternion.h"
#include "VertexTypes.h"

#include <d3dcompiler.h>
#include <GeometricPrimitive.h>
#include <GraphicsMemory.h> // GeometricPrimitive.h

class RootSignatureCommon;

using Microsoft::WRL::ComPtr;
class Teapot
{
public:
    Teapot(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, RootSignatureCommon* rootSignature);
    void RecordCommandList(ID3D12GraphicsCommandList* commandList);

private:
    ComPtr<ID3D12PipelineState> m_pipelineState;

    std::unique_ptr<DirectX::GeometricPrimitive> m_teapot;
    Framework::Vqs m_transform;
};
