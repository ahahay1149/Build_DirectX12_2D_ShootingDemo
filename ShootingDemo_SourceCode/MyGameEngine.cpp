#include "MyGameEngine.h"
#include <directxcolors.h>  

#include "D3D11Helper.h"

#include "DXSampleHelper.h"

#include "InputManager.h"
#include "SoundManager.h"
#include "GameObjectManager.h"
#pragma comment(lib, "d3d12.lib")           
#pragma comment(lib, "dxgi.lib")            
using namespace DirectX;                    

void MyGameEngine::WaitForGpu()
{
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    m_fenceValues[m_frameIndex]++;
}

void MyGameEngine::MoveToNextFrame()
{
    const UINT64 currentFenceVal = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceVal));

    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    m_fenceValues[m_frameIndex] = currentFenceVal + 1;
}
void MyGameEngine::Render()
{
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset()); 
    ThrowIfFailed(m_commandLists[m_frameIndex]->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

    m_commandLists[m_frameIndex]->RSSetViewports(1, &m_viewport);
    m_commandLists[m_frameIndex]->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandLists[m_frameIndex]->ResourceBarrier(1, &tra);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    m_commandLists[m_frameIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    m_commandLists[m_frameIndex]->ClearRenderTargetView(rtvHandle, Colors::MidnightBlue, 0, nullptr);
    m_commandLists[m_frameIndex]->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &m_scissorRect);

    tra = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandLists[m_frameIndex]->ResourceBarrier(1, &tra);

    ThrowIfFailed(m_commandLists[m_frameIndex]->Close());

    ID3D12CommandList* ppCommandLists[] = { m_commandLists[m_frameIndex].Get(), m_pSpriteRenderer->RenderSprites(this) };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


    ThrowIfFailed( m_pSwapChain->Present(1, 0) );
}

MyGameEngine::MyGameEngine()
{
}

void MyGameEngine::setEngineWH(UINT width, UINT height, std::wstring title)
{
    m_title = title;
    m_windowWidth = width;
    m_windowHeight = height;
}
HRESULT MyGameEngine::initMyGameEngine(HINSTANCE hInstance, HWND hwnd)
{
    HRESULT hr = S_OK;  

    RECT rc;
    GetClientRect(hwnd, &rc);   
    UINT width = rc.right - rc.left;    
    UINT height = rc.bottom - rc.top;

    UINT dxgiFactoryFlags = 0;      

#ifdef _DEBUG
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

            ComPtr<ID3D12Debug1> spDebugController1;
            debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));
            spDebugController1->SetEnableGPUBasedValidation(true);

        }
    }

#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_2,          
        D3D_FEATURE_LEVEL_12_1,      
        D3D_FEATURE_LEVEL_12_0,      

        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    {
        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;

        if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference( 
                    adapterIndex,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,    
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)    
                {
                    continue;
                }

                hr = E_FAIL;

                for (UINT featureLevel = 0; featureLevel < numFeatureLevels; featureLevel++)
                {
                    m_featureLevel = featureLevels[featureLevel];
                    if (SUCCEEDED(hr = D3D12CreateDevice(adapter.Get(), m_featureLevel, _uuidof(ID3D12Device), &m_pd3dDevice)))
                    {
                        break;
                    }

                }

                if (SUCCEEDED(hr))
                    break;
            }
        }
        
        if (m_pd3dDevice.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)    
                {
                    continue;
                }

                hr = E_FAIL;

                for (UINT featureLevel = 0; featureLevel < numFeatureLevels; featureLevel++)
                {
                    m_featureLevel = featureLevels[featureLevel];
                    if (SUCCEEDED(hr = D3D12CreateDevice(adapter.Get(), m_featureLevel, _uuidof(ID3D12Device), &m_pd3dDevice)))
                    {
                        break;
                    }

                }

                if (SUCCEEDED(hr))
                    break;
            }

        }

    }

    ThrowIfFailed(hr);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));



    ComPtr<IDXGISwapChain1> swapChain;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;   
    swapChainDesc.SampleDesc.Count = 1;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),                      
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    ThrowIfFailed(swapChain.As(&m_pSwapChain)); 
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();   

    ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.NodeMask = 0;                               
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;      
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;    
        ThrowIfFailed(m_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    }

    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());   

        for (UINT n = 0; n < FRAME_COUNT; n++)
        {
            ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));   
            m_pd3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle); 
            rtvHandle.Offset(1, m_rtvDescriptorSize);                                   
            NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);                              

            ThrowIfFailed(m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }
    }


    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};    
        dsvHeapDesc.NumDescriptors = 1;  
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ThrowIfFailed(m_pd3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    }

    {
        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        const CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(m_pd3dDevice->CreateCommittedResource(
            &depthStencilHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilTextureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_pDepthStencil)
        ));

        NAME_D3D12_OBJECT(m_pDepthStencil); 

        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }




    CD3DX12_HEAP_PROPERTIES upheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    CD3DX12_RESOURCE_DESC buffDesc = CD3DX12_RESOURCE_DESC::Buffer(((sizeof(XMMATRIX) * FRAME_COUNT) + 255) & ~255);     
    ThrowIfFailed(m_pd3dDevice->CreateCommittedResource(
        &upheap,
        D3D12_HEAP_FLAG_NONE,
        &buffDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pCBViewMatrix)));

    NAME_D3D12_OBJECT(m_pCBViewMatrix); 

    ThrowIfFailed(m_pd3dDevice->CreateCommittedResource(
        &upheap,
        D3D12_HEAP_FLAG_NONE,
        &buffDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pCBProjectionMatrix)));

    NAME_D3D12_OBJECT(m_pCBProjectionMatrix); 

    XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -20.0f, 0.0f);    
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);      
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);       
    XMMATRIX ViewMat = XMMatrixLookAtLH(Eye, At, Up);       
    XMMATRIX cbView = XMMatrixTranspose(ViewMat);
    {
        uint8_t* memory = nullptr;
        m_pCBViewMatrix->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, &cbView, sizeof(XMMATRIX));
        m_pCBViewMatrix->Unmap(0, nullptr);
    }

    XMMATRIX ProjectionMat = XMMatrixPerspectiveFovLH(atan2(static_cast<float>(height) * 0.25f, static_cast<float>(At.m128_f32[2] - Eye.m128_f32[2])) * 2.0f, (FLOAT)width / (FLOAT)height, 0.01f, 100.0f);
    XMMATRIX cbProjection  = XMMatrixTranspose(ProjectionMat);
    {
        uint8_t* memory = nullptr;
        m_pCBProjectionMatrix->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, &cbProjection, sizeof(XMMATRIX));
        m_pCBProjectionMatrix->Unmap(0, nullptr);
    }

    m_viewport.Width = (FLOAT)width;
    m_viewport.Height = (FLOAT)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;

    m_scissorRect.top = 0;
    m_scissorRect.bottom = height;
    m_scissorRect.left = 0;
    m_scissorRect.right = width;

    for (int i = 0; i < FRAME_COUNT; i++)
    {
        ThrowIfFailed(m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), nullptr, IID_PPV_ARGS(&m_commandLists[i])));
        ThrowIfFailed(m_commandLists[i]->Close());
    }

    {
       ThrowIfFailed(m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
       for (int i = 0; i < FRAME_COUNT; i++)
       {
           m_fenceValues[i] = 1;
       }

       m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
       if (m_fenceEvent == nullptr)
       {
           ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
       }
    }

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return hr;
    m_pTextureMng.reset(new TextureManager());
    hr = m_pTextureMng->initTextureManager();
    if (FAILED(hr))
        return hr;

    m_pSpriteRenderer.reset(new MySpriteRenderer());
    hr = m_pSpriteRenderer->initMySpriteRenderer(m_pd3dDevice.Get());
    if (FAILED(hr))
        return hr;
    if (!QueryPerformanceFrequency(&m_timerFreq))
    {
        return E_FAIL;
    }
    InputManager* inputMng = InputManager::getInstance();
    inputMng->initInputManager(hInstance, hwnd);
    SoundManager* soundMng = SoundManager::getInstance();
    soundMng->initSoundManager();
    m_pSceneController.reset(new SceneController());
    m_pSceneController->initSceneController();

    return S_OK;
}

HRESULT MyGameEngine::loadTextureFile(int index, const wchar_t* filename)
{
    return m_pTextureMng->createTextureFromFile(m_pd3dDevice.Get(), index, filename);
}

HRESULT MyGameEngine::uploadTexture()
{
    m_commandLists[m_frameIndex]->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
    return m_pTextureMng->uploadCreatedTextures(m_pd3dDevice.Get(), m_commandLists[m_frameIndex].Get(), m_commandQueue.Get());
}

void MyGameEngine::setMainRenderTarget(ID3D12GraphicsCommandList* cmdList)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    cmdList->RSSetViewports(1, &m_viewport);
    cmdList->RSSetScissorRects(1, &m_scissorRect);

}

ID3D12Resource* MyGameEngine::getRenderTarget(int frameIndex)
{
    return m_renderTargets[frameIndex].Get();
}

void MyGameEngine::FrameUpdate()
{

    LARGE_INTEGER   nowTimer;
    float           frame;
    const float     FRAME_TIME = 1.0f / 60.0f;

    QueryPerformanceCounter(&nowTimer);
    frame = static_cast<float>(nowTimer.QuadPart - m_preTimer.QuadPart) / static_cast<float>(m_timerFreq.QuadPart);

    if (frame < FRAME_TIME)
        return; 

    m_preTimer = nowTimer;
    InputManager* inputMng = InputManager::getInstance();
    inputMng->update();
    GameObjectManager::getInstance()->action();
    Render();

    inputMng->refreshBuffer();
    SoundManager* soundMng = SoundManager::getInstance();
    soundMng->audioUpdate();
    MoveToNextFrame();
}

void MyGameEngine::CleanupDevice()
{
    WaitForGpu();
    CloseHandle(m_fenceEvent);

    GameObjectManager::getInstance()->cleanupGameObjectManager();

    CoUninitialize();
    m_pSpriteRenderer.reset();
    m_pSceneController.reset();
    SoundManager* soundMng = SoundManager::getInstance();
    soundMng->destructSoundManager();
    m_pTextureMng.reset();
    m_pCBProjectionMatrix = nullptr;
    m_pCBViewMatrix = nullptr;
    m_pDepthStencil = nullptr;
    m_pSwapChain = nullptr;
    m_pd3dDevice = nullptr;
}
