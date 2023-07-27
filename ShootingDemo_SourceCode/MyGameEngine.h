#pragma once

#include "stdafx.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define FRAME_COUNT (2)

#include "MySpriteRenderer.h"   
#include <memory>
#include "TextureManager.h"
#include "SceneController.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class MySpriteRenderer;

class MyGameEngine
{

private:
    D3D_DRIVER_TYPE                     m_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL                   m_featureLevel = D3D_FEATURE_LEVEL_12_1;
    ComPtr<ID3D12Device2> m_pd3dDevice = nullptr;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    ComPtr<ID3D12GraphicsCommandList> m_commandLists[FRAME_COUNT];
    ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr;
    ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
    ComPtr<ID3D12Resource> m_pDepthStencil = nullptr;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;      
    ComPtr<ID3D12DescriptorHeap> m_smpHeap;      
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;      
    UINT m_rtvDescriptorSize;                    

    ComPtr<ID3D12Resource> m_pCBViewMatrix = nullptr;
    ComPtr<ID3D12Resource> m_pCBProjectionMatrix = nullptr;
    std::unique_ptr<MySpriteRenderer> m_pSpriteRenderer = nullptr;  
    LARGE_INTEGER   m_timerFreq;
    LARGE_INTEGER   m_preTimer;
    std::wstring    m_title;
    UINT            m_windowWidth;
    UINT            m_windowHeight;
    std::unique_ptr<TextureManager> m_pTextureMng = nullptr;
    std::unique_ptr<SceneController> m_pSceneController = nullptr;
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FRAME_COUNT];
    void Render();

    MyGameEngine();
public:

    void setEngineWH(UINT, UINT, std::wstring);
    const WCHAR* getTitle() { return m_title.c_str(); }

    UINT getWidth() const { return m_windowWidth; }
    UINT getHeight() const { return m_windowHeight; }
    void WaitForGpu();
    void MoveToNextFrame();
    static MyGameEngine* getInstance()
    {
        static MyGameEngine* engine = nullptr;

        if (engine == nullptr)
        {
            engine = new MyGameEngine();
        }

        return engine;
    }
    HRESULT initMyGameEngine(HINSTANCE, HWND);

    TextureManager* getTextureManager()
    {
        return m_pTextureMng.get();
    }

    HRESULT loadTextureFile(int index, const wchar_t* filename);

    HRESULT uploadTexture();
    void setMainRenderTarget(ID3D12GraphicsCommandList* cmdList);
    ID3D12Resource* getRenderTarget(int frameIndex);

    MySpriteRenderer* getSpriteRenderer()
    {
        return m_pSpriteRenderer.get();
    }

    void FrameUpdate();
    void CleanupDevice();

    ID3D12CommandAllocator* getCommandAllocator()
    {
        return m_commandAllocators[m_frameIndex].Get();
    }

    ID3D12CommandQueue* getCommandQueue()
    {
        return m_commandQueue.Get();
    }

    ID3D12Resource* getViewMatrix()
    {
        return m_pCBViewMatrix.Get();
    }

    ID3D12Resource* getProjectionMatrix()
    {
        return m_pCBProjectionMatrix.Get();
    }

    ID3D12Device2* getD3DDevice()
    {
        return m_pd3dDevice.Get();
    }

    UINT getFrameIndex()
    {
        return m_frameIndex;
    }
};

