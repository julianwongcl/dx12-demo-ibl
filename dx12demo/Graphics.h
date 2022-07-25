#pragma once

#include <DirectXMath.h>
#include <dxgi1_4.h> // IDXGISwapChain3
#include <GraphicsMemory.h>
#include <SimpleMath.h>
#include <Windows.h>
#include <wrl.h>

#include "d3dx12.h"

#include "Camera.h"
#include "ConstantBuffers.h"
#include "HammersleySequence.h"

class DescHeapSRVCommon;
class Dx12Quad;
class Inputs;
class RootSignatureCommon;
class Sky;
class Teapot;
class Textures;

using Microsoft::WRL::ComPtr;
class Graphics
{
public:
    Graphics(HWND windowHandle, Inputs* inputs, LONG width, LONG height);
    ~Graphics(void);

    void Update(double dt);
    void WaitForGPU();
    void MoveToNextFrame();

    static const UINT64 K_FRAMECOUNT = 2;

private:
    Inputs* m_inputs = nullptr;

    // WVP
    CD3DX12_RECT                m_scissorRect;
    CD3DX12_VIEWPORT            m_viewport;
    DirectX::SimpleMath::Matrix m_perspectiveFOV;
    DirectX::SimpleMath::Matrix m_view;
    Camera                      m_camera;
    ComPtr<ID3D12Resource>      m_cbUploadHeap;
    ConstBuffWVP                m_constBuffWVP;
    UINT8*                      m_cbUploadHeapBegin = nullptr;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3>           m_swapChain;
    ComPtr<ID3D12Device>              m_device;
    ComPtr<ID3D12Resource>            m_renderTargets[K_FRAMECOUNT];
    ComPtr<ID3D12CommandAllocator>    m_commandAllocators[K_FRAMECOUNT];
    ComPtr<ID3D12CommandQueue>        m_commandQueue;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12DescriptorHeap>      m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap>      m_cbvHeap;
    ComPtr<ID3D12PipelineState>       m_pipelineState;
    UINT m_rtvDescriptorSize;

    RootSignatureCommon* m_rootSignature = nullptr;
    DescHeapSRVCommon*   m_descHeapSRV   = nullptr;

    // resources
    ComPtr<ID3D12Resource>       m_depthStencil;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    // Synchronization objects.
    UINT                m_frameIndex = 0;
    HANDLE              m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64              m_fenceValues[K_FRAMECOUNT] {};

    // DXTK12
    GraphicsMemory*     m_graphicsMemory = nullptr;
    HammersleySequence* m_hammersleySeq  = nullptr;
    Sky*                m_sky            = nullptr;
    Teapot*             m_teapot         = nullptr;
    Textures*           m_textures       = nullptr;
};
