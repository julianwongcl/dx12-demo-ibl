#include "Graphics.h"

#include "DescHeapSRVCommon.h"
#include "ExceptionHelper.h"
#include "HardwareAdapterHelper.h"
#include "RootSignatureCommon.h"
#include "Sky.h"
#include "Teapot.h"
#include "Textures.h"

#include <iostream> // runtime_error

using namespace Microsoft::WRL;

Graphics::Graphics(HWND windowHandle, Inputs* inputs, LONG width, LONG height)
    :
    m_inputs(inputs)
{
    HRESULT hr;
    UINT dxgiFactoryFlags = 0;

    {
        m_scissorRect = CD3DX12_RECT(0, 0, width, height);
        const float w = static_cast<float>(width);
        const float h = static_cast<float>(height);
        m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, w, h);
        const float fov = (3.14159f) / 3.0f;
        const float aspect_ratio = w / h;
        const float plane_near = 0.01f;
        const float plane_far = 1000.0f;
        m_perspectiveFOV = DirectX::SimpleMath::Matrix::
            CreatePerspectiveFieldOfView(fov, aspect_ratio, plane_near, plane_far);
        m_constBuffWVP.mtx_proj = m_perspectiveFOV;
    }

    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);
        hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        ThrowIfFailed(hr);
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = K_FRAMECOUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    hr = factory->CreateSwapChainForHwnd(m_commandQueue.Get(),
        windowHandle,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain);
    ThrowIfFailed(hr);

    // disable alt-enter fullscreen
    ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = K_FRAMECOUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        ThrowIfFailed(hr);
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        // wvp matrices
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        // Create a RTV for each frame.
        for (UINT n = 0; n < K_FRAMECOUNT; ++n)
        {
            hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            ThrowIfFailed(hr);
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            // create command allocator
            hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_commandAllocators[n]));
            ThrowIfFailed(hr);
        }
    }

    // Create the depth stencil view.
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        static const auto HeapPropertyDefaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const auto& ResourceDescTex2DDepthStencil =
            CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
                width, height, 1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        hr = m_device->CreateCommittedResource(&HeapPropertyDefaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDescTex2DDepthStencil,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_depthStencil));
        ThrowIfFailed(hr);

        m_device->CreateDepthStencilView(m_depthStencil.Get(),
            &depthStencilDesc,
            m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the constant buffer.
    {
        // 256-byte aligned
        const UINT constantBufferSize = sizeof(ConstBuffWVP);
        static const auto HeapPropertyUploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto ResourceDescConstBufferPerEntity = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
        hr = m_device->CreateCommittedResource(&HeapPropertyUploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDescConstBufferPerEntity,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_cbUploadHeap));
        ThrowIfFailed(hr);

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_cbUploadHeap->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;
        m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        // We do not intend to read from this resource on the CPU.
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(m_cbUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_cbUploadHeapBegin)));

        // create hammersley sequence
        m_hammersleySeq = new HammersleySequence();
        memcpy(m_constBuffWVP.seq_hammersley, m_hammersleySeq->sequence, sizeof(m_hammersleySeq->sequence));
        memcpy(m_cbUploadHeapBegin, &m_constBuffWVP, sizeof(m_constBuffWVP));
    }

    // create a command list
    hr = m_device->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[m_frameIndex].Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList));
    ThrowIfFailed(hr);

    // initialize DXTK12
    m_graphicsMemory = new GraphicsMemory(m_device.Get());

    // load assets
    m_descHeapSRV = new DescHeapSRVCommon(m_device.Get(), m_commandList.Get());
    m_textures = new Textures(m_device.Get(), m_commandList.Get(), m_descHeapSRV);

    m_rootSignature = new RootSignatureCommon(m_device.Get(), m_commandList.Get());
    m_sky = new Sky(m_device.Get(), m_commandList.Get(), m_rootSignature);
    m_teapot = new Teapot(m_device.Get(), m_commandList.Get(), m_rootSignature);

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());

    // execute uploads
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // create a fence
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex],
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_fence)));
        ++m_fenceValues[m_frameIndex];

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    WaitForGPU();
}

Graphics::~Graphics(void)
{
    delete m_hammersleySeq;
    delete m_teapot;
    delete m_sky;
    delete m_textures;
    delete m_descHeapSRV;
    delete m_rootSignature;
    delete m_graphicsMemory;
}

void Graphics::Update(double dt)
{
    HRESULT hr;

    // sky and teapot both share this
    // but they should both have their own
    auto vqs = Framework::Vqs();
    vqs.s = 1.0f;

    // set up WVP
    m_camera.Update(dt, m_inputs);
    m_view = m_camera.GetViewMatrix();
    m_constBuffWVP.mtx_view = m_view;
    m_constBuffWVP.mtx_viewinv = DirectX::XMMatrixInverse(nullptr, m_view);
    m_constBuffWVP.mtx_world = vqs.ToMatrix();
    memcpy(m_cbUploadHeapBegin, &m_constBuffWVP, sizeof(m_constBuffWVP));

    // render
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    hr = m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
    ThrowIfFailed(hr);
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // transition the resource barrier for the backbuffer from Present to RT
    auto presentToRt = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &presentToRt);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex,
        m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_commandList->SetGraphicsRootSignature(m_rootSignature->rootSignatureCommon);

    ID3D12DescriptorHeap* srvHeap = m_descHeapSRV->descHeapSrvCommon;
    ID3D12DescriptorHeap* ppHeaps[] = { srvHeap };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_cbUploadHeap->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

    // draw the sky and the teapot
    m_sky->RecordCommandList(m_commandList.Get());
    m_teapot->RecordCommandList(m_commandList.Get());

    // Indicate that the back buffer will now be used to present.
    auto rtToPresent = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &rtToPresent);
    m_commandList->Close();

    // execute all command lists
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    MoveToNextFrame();

    // inform DXTK12 of present
    m_graphicsMemory->Commit(m_commandQueue.Get());
}

void Graphics::WaitForGPU()
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    ++m_fenceValues[m_frameIndex];
}

void Graphics::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}
