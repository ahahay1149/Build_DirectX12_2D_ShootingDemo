#include "MySpriteRenderer.h"
#include "D3D11Helper.h"	

#include "DXSampleHelper.h" 

#include "MyGameEngine.h"
#include "TextureManager.h"

struct SpriteVertex
{
    XMFLOAT3 Pos;   
    XMFLOAT2 Tex;   
};
HRESULT MySpriteRenderer::initMySpriteRenderer(ID3D12Device2 * d3dDevice)
{

    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) 
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        CD3DX12_ROOT_PARAMETER1 rootParameters[8];

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); 
        rootParameters[0].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[1].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[3].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[4].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[5].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[6].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[7].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    {
        ID3DBlob* pVertexBlob = nullptr;    

        HRESULT hr = CompileShaderFromFile(L"shaders/SpriteVertexShader.hlsl", "main", "vs_4_1", &pVertexBlob);
        if (FAILED(hr))
        {
            MessageBox(nullptr,
                L"頂点シェーダのコンパイルに失敗しました。", L"Error", MB_OK);
            return hr;
        }

        ID3DBlob* pPixelBlob = nullptr;    

        hr = CompileShaderFromFile(L"shaders/ColorMixPixelShader.hlsl", "main", "ps_4_1", &pPixelBlob);
        if (FAILED(hr))
        {
            MessageBox(nullptr,
                L"ピクセルシェーダのコンパイルに失敗しました。", L"Error", MB_OK);
            return hr;
        }

        D3D12_INPUT_ELEMENT_DESC layout[] =  
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };


        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { layout, _countof(layout) };
        psoDesc.pRootSignature = m_rootSignature.Get();

        psoDesc.VS.BytecodeLength = pVertexBlob->GetBufferSize();
        psoDesc.PS.BytecodeLength = pPixelBlob->GetBufferSize();
        psoDesc.VS.pShaderBytecode = pVertexBlob->GetBufferPointer();
        psoDesc.PS.pShaderBytecode = pPixelBlob->GetBufferPointer();

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);          
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;   
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.StencilEnable = TRUE;      
        psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
        psoDesc.DepthStencilState.BackFace = defaultStencilOp;

        psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        
        ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

        pVertexBlob->Release();
        pPixelBlob->Release();
    }


    SpriteVertex vertices[] =        
    {
        { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0, 1) },   
        { XMFLOAT3(0.5f, -0.5f, 1.0f), XMFLOAT2(1, 1) },    
        { XMFLOAT3(0.5f, 0.5f, 1.0f), XMFLOAT2(1, 0) },     
        { XMFLOAT3(-0.5f, 0.5f, 1.0f), XMFLOAT2(0, 0) },    
    };

    WORD indices[] =  
    {
        1,0,2,
        0,3,2,
    };

    XMFLOAT4 uvData = {};
    XMFLOAT2 sizeData = {};
    CD3DX12_HEAP_PROPERTIES upheapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    CD3DX12_RESOURCE_DESC sourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upheapProp,   
        D3D12_HEAP_FLAG_NONE,
        &sourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pSpriteVertexBuffer)));

    {
        uint8_t* memory = nullptr;
        m_pSpriteVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, vertices, sizeof(vertices));
        m_pSpriteVertexBuffer->Unmap(0, nullptr);
    }

    NAME_D3D12_OBJECT(m_pSpriteVertexBuffer); 

    m_vertexBufferView.BufferLocation = m_pSpriteVertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(vertices);
    m_vertexBufferView.StrideInBytes = sizeof(SpriteVertex);


    CD3DX12_HEAP_PROPERTIES upHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(WORD) * 6);

    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pSpriteIndexBuffer)));

    {
        uint8_t* memory = nullptr;
        m_pSpriteIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, indices, sizeof(WORD) * 6);
        m_pSpriteIndexBuffer->Unmap(0, nullptr);
    }

    NAME_D3D12_OBJECT(m_pSpriteIndexBuffer); 

    m_indexBufferView.BufferLocation = m_pSpriteIndexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = sizeof(WORD) * 6;

    UINT frameOffset = FRAME_COUNT * MAX_SPRITES;
    UINT buffSize = (sizeof(XMFLOAT4) + 255) & ~255;

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize * frameOffset);
    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pUvBuffer)));

    NAME_D3D12_OBJECT(m_pUvBuffer); 

    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pSizeBuffer)));

    NAME_D3D12_OBJECT(m_pSizeBuffer); 


    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pColorBuffer)));

    NAME_D3D12_OBJECT(m_pColorBuffer); 

    buffSize = (sizeof(MixSettings) + 255) & ~255;
    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize * frameOffset);
    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pMixBuffer)));

    NAME_D3D12_OBJECT(m_pMixBuffer); 

    buffSize = (sizeof(XMMATRIX) + 255) & ~255;
    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize * frameOffset);
    ThrowIfFailed(d3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pAffineMatrix)));

    NAME_D3D12_OBJECT(m_pAffineMatrix); 

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 9;         
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));


    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    UINT constantBufferSize = (sizeof(XMMATRIX) + 255) & ~255;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHndl = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT offsetSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    cbvDesc.SizeInBytes = constantBufferSize;   
    cbvDesc.BufferLocation = m_pAffineMatrix->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    constantBufferSize = (sizeof(XMFLOAT2) + 255) & ~255;
    cbvDesc.SizeInBytes = constantBufferSize;   
    cbvDesc.BufferLocation = m_pSizeBuffer->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    constantBufferSize = (sizeof(XMFLOAT4) + 255) & ~255;
    cbvDesc.SizeInBytes = constantBufferSize;   
    cbvDesc.BufferLocation = m_pUvBuffer->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    cbvDesc.SizeInBytes = constantBufferSize;   
    cbvDesc.BufferLocation = m_pColorBuffer->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    cbvDesc.SizeInBytes = (sizeof(MixSettings) + 255) & ~255;   
    cbvDesc.BufferLocation = m_pMixBuffer->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    constantBufferSize = (sizeof(XMMATRIX) + 255) & ~255;
    MyGameEngine* engine = MyGameEngine::getInstance();
    cbvDesc.BufferLocation = engine->getViewMatrix()->GetGPUVirtualAddress();   
    cbvDesc.SizeInBytes = constantBufferSize;
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);
    cpuHndl.ptr += offsetSize;

    cbvDesc.BufferLocation = engine->getProjectionMatrix()->GetGPUVirtualAddress();
    d3dDevice->CreateConstantBufferView(&cbvDesc, cpuHndl);                     
    cpuHndl.ptr += offsetSize;

    m_spriteList.clear();
    return S_OK;
}

void MySpriteRenderer::addMySprite(MySprite* mysp)
{
    m_spriteList.push_back(mysp);
}

ID3D12CommandList* MySpriteRenderer::RenderSprites(MyGameEngine* myEngine)
{
    ID3D12CommandAllocator* cmdAl = myEngine->getCommandAllocator();
    ID3D12CommandQueue* queue = myEngine->getCommandQueue();

    ID3D12Resource* p_viewMtx = myEngine->getViewMatrix();
    ID3D12Resource* p_prjMtx = myEngine->getProjectionMatrix();

    UINT frameIndex = myEngine->getFrameIndex();

    if (m_commandLists[frameIndex] == nullptr)
    {
        ThrowIfFailed(myEngine->getD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAl, m_pipelineState.Get(), IID_PPV_ARGS(&m_commandLists[frameIndex])));
        m_commandLists[frameIndex]->Close();
    }

    ID3D12GraphicsCommandList* cmdList = m_commandLists[frameIndex].Get();

    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipelineState.Get()));

    myEngine->setMainRenderTarget(cmdList);

    CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->getRenderTarget(frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &tra);

    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    cmdList->SetGraphicsRootConstantBufferView(5, p_viewMtx->GetGPUVirtualAddress());　
    cmdList->SetGraphicsRootConstantBufferView(6, p_prjMtx->GetGPUVirtualAddress());　

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->IASetIndexBuffer(&m_indexBufferView);

    TextureManager* texMng = MyGameEngine::getInstance()->getTextureManager();

    int drawCount = 0;
    int frmIndex = myEngine->getFrameIndex() * MAX_SPRITES;

    int lastTexId = -1;

    UINT affinBuffSize = (sizeof(XMMATRIX) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS affineAddr = m_pAffineMatrix->GetGPUVirtualAddress() + affinBuffSize * frmIndex;

    UINT f4BuffSize = (sizeof(XMFLOAT4) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS uvAddr = m_pUvBuffer->GetGPUVirtualAddress() + f4BuffSize * frmIndex;
    D3D12_GPU_VIRTUAL_ADDRESS colorAddr = m_pColorBuffer->GetGPUVirtualAddress() + f4BuffSize * frmIndex;

    UINT f2BuffSize = (sizeof(XMFLOAT2) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS sizeAddr = m_pSizeBuffer->GetGPUVirtualAddress() + f2BuffSize * frmIndex;

    UINT mixBuffSize = (sizeof(MixSettings) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS mixAddr = m_pMixBuffer->GetGPUVirtualAddress() + mixBuffSize * frmIndex;

    for (auto p_sprite : m_spriteList)
    {
        cmdList->SetGraphicsRootConstantBufferView(0, affineAddr + affinBuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(1, uvAddr + f4BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(2, sizeAddr + f2BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(3, colorAddr + f4BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(4, mixAddr + mixBuffSize * drawCount);

        const XMFLOAT4* p_rect = p_sprite->getUV();
        m_aUv[drawCount] = *p_rect;

        const XMFLOAT2* p_size = p_sprite->getSize();
        m_aSize[drawCount] = *p_size;
        const XMFLOAT4* p_color = p_sprite->getColor();

        m_aColor[drawCount] = *p_color;
        m_aMix[drawCount] = { p_sprite->getColorMix(), p_sprite->getAlphaMix() };

        const XMFLOAT3& position = *p_sprite->getPosition();
        const XMFLOAT3& scale = *p_sprite->getScale();
        const XMFLOAT3& rotate = *p_sprite->getRotation();

        XMMATRIX AffineMat = XMMatrixIdentity();
        AffineMat *= XMMatrixScaling(scale.x, scale.y, scale.z); 
        AffineMat *= XMMatrixRotationX(rotate.x);                
        AffineMat *= XMMatrixRotationY(rotate.y);
        AffineMat *= XMMatrixRotationZ(rotate.z);
        AffineMat *= XMMatrixTranslation(position.x, position.y, position.z);    

        AffineMat = XMMatrixTranspose(AffineMat); 

        m_aAffin[drawCount] = AffineMat;

        int texId = p_sprite->getTexId();
        if (lastTexId != texId)
        {
            texMng->setTexture(myEngine->getD3DDevice(), m_srvHeap.Get(), 7, texId);
            lastTexId = texId;
        }

        auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
        gpuHeap.ptr += myEngine->getD3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (7 + texId);
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(gpuHeap);
        cmdList->SetGraphicsRootDescriptorTable(7, cbvSrvGpuHandle);    　

        cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

        drawCount++;
    }


    tra = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->getRenderTarget(frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &tra);

    m_spriteList.clear();
    cmdList->Close();

    {
        UINT8* size_mapBuff;
        ThrowIfFailed(m_pSizeBuffer->Map(0, nullptr, reinterpret_cast<void**>(&size_mapBuff)));

        UINT8* uv_mapBuff;
        ThrowIfFailed(m_pUvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&uv_mapBuff)));

        UINT8* color_mapBuff;
        ThrowIfFailed(m_pColorBuffer->Map(0, nullptr, reinterpret_cast<void**>(&color_mapBuff)));

        UINT8* mix_mapBuff;
        ThrowIfFailed(m_pMixBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mix_mapBuff)));

        UINT8* affin_mapBuff;
        ThrowIfFailed(m_pAffineMatrix->Map(0, nullptr, reinterpret_cast<void**>(&affin_mapBuff)));

        for (int dc = 0; dc < drawCount; dc++)
        {
            memcpy(size_mapBuff + (frmIndex + dc) * f2BuffSize, &m_aSize[dc], sizeof(XMFLOAT2));
            memcpy(uv_mapBuff + (frmIndex + dc) * f4BuffSize, &m_aUv[dc], sizeof(XMFLOAT4));
            memcpy(color_mapBuff + (frmIndex + dc) * f4BuffSize, &m_aColor[dc], sizeof(XMFLOAT4));
            memcpy(mix_mapBuff + (frmIndex + dc) * mixBuffSize, &m_aMix[dc], sizeof(MixSettings));
            memcpy(affin_mapBuff + (frmIndex + dc) * affinBuffSize, &m_aAffin[dc], sizeof(XMMATRIX));
        }

        m_pSizeBuffer->Unmap(0, nullptr);
        m_pUvBuffer->Unmap(0, nullptr);
        m_pColorBuffer->Unmap(0, nullptr);
        m_pMixBuffer->Unmap(0, nullptr);
        m_pAffineMatrix->Unmap(0, nullptr);
    }

    return cmdList;
}

MySpriteRenderer::~MySpriteRenderer()
{
    destructMySpriteRenderer();
}

void MySpriteRenderer::destructMySpriteRenderer()
{
    m_spriteList.clear();
    m_pSpriteVertexBuffer = nullptr;		
    m_pSpriteIndexBuffer = nullptr;        

    m_pUvBuffer = nullptr;					
    m_pSizeBuffer = nullptr;				
    m_pMixBuffer = nullptr;

    m_pAffineMatrix = nullptr;				

    for (int i = 0; i < FRAME_COUNT; i++)
        m_commandLists[i] = nullptr;

    m_pipelineState = nullptr;

}