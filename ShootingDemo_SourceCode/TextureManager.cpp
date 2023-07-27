#include <Windows.h>
#include <vector>
#include "TextureManager.h"
#include <WICTextureLoader.h>	 
#include "DXSampleHelper.h" 
#include "d3dx12.h"			


#pragma comment(lib, "DirectXTK12.lib")

using namespace std;
using namespace DirectX;


HRESULT TextureManager::initTextureManager(void)
{
	int i;

	for (i = 0; i < MAX_TEXTURES; i++)
	{
		releaseTexture(i);
	}

	return S_OK;
}

void TextureManager::releaseTexObj(Texture2DContainer* txbuff)
{
	txbuff->m_pTexture.Reset();
	txbuff->m_wicData.reset();
	txbuff->m_pTextureUploadHeap.Reset();

	txbuff->m_uploaded = false;
}

void	TextureManager::destructTextureManager(void)
{
	releaseAllTextures();
}

HRESULT TextureManager::createTextureFromFile(ID3D12Device* pD3D, int index, const wchar_t* filename)
{
	HRESULT hr;

	if (index >= MAX_TEXTURES)
		return E_FAIL;

	hr = createTextureFromFile(pD3D, &m_textureArray[index], filename);

	return hr;
}

HRESULT TextureManager::uploadCreatedTextures(ID3D12Device* pD3D, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandQueue* pCmdQueue)
{
	HRESULT hres = E_FAIL;

	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		Texture2DContainer* txbuff = &m_textureArray[i];

		if (!txbuff->m_uploaded) continue;

		const UINT subresoucesize = 1;	

		UpdateSubresources(pCmdList,
			txbuff->m_pTexture.Get(),
			txbuff->m_pTextureUploadHeap.Get(),
			0,
			0,
			subresoucesize,
			&txbuff->m_subresouceData);

		CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(txbuff->m_pTexture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		pCmdList->ResourceBarrier(1, &tra);

		pCmdList->DiscardResource(txbuff->m_pTextureUploadHeap.Get(), nullptr);

		txbuff->m_uploaded = false;

	}

	ThrowIfFailed(hres = pCmdList->Close());
	ID3D12CommandList* ppCommandLists[] = { pCmdList };
	pCmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return hres;
}

void TextureManager::releaseTexture(int index)
{
	if (index < MAX_TEXTURES)
	{
		releaseTexObj(&m_textureArray[index]);
	}
}

void TextureManager::releaseAllTextures(void)
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		releaseTexture(i);
	}
}

void TextureManager::setTexture(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, UINT texIndex)
{
	if (texIndex < MAX_TEXTURES)
	{
		if (m_textureArray[texIndex].m_pTexture != NULL && !m_textureArray[texIndex].m_viewCreated)
			setTexture(pD3D, pSrvHeap, slotNo + texIndex, &m_textureArray[texIndex]);
	}
}

void TextureManager::unsetTexture(ID3D12CommandList* pCommandList)
{
}

void TextureManager::setTexture(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, Texture2DContainer* txbuff)
{


	UINT offset = pD3D->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	size_t startAddr = pSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = txbuff->texFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	heapHandle.ptr = startAddr + offset * slotNo;

	pD3D->CreateShaderResourceView(txbuff->m_pTexture.Get(),
		&srvDesc,
		heapHandle);

	txbuff->m_viewCreated = true;
  
}

Texture2DContainer* TextureManager::getTexture(int index)
{
	if (index < MAX_TEXTURES)
	{
		if (m_textureArray[index].m_pTexture != nullptr)
			return (&m_textureArray[index]);
	}

	return nullptr;
}

TextureManager::~TextureManager()
{
	destructTextureManager();
}

HRESULT TextureManager::createTextureFromFile(ID3D12Device* pD3D, Texture2DContainer* txbuff, const wchar_t* filename, bool genMipmap)
{
	HRESULT hres;
	releaseTexObj(txbuff);

	hres = LoadWICTextureFromFile(pD3D, filename, (ID3D12Resource**)txbuff->m_pTexture.GetAddressOf(), txbuff->m_wicData, txbuff->m_subresouceData);
	if (SUCCEEDED(hres))
	{

		D3D12_RESOURCE_DESC texDesc;
		texDesc = txbuff->m_pTexture->GetDesc();
		txbuff->fWidth = static_cast<float>(texDesc.Width);
		txbuff->fHeight = static_cast<float>(texDesc.Height);
		txbuff->texFormat = texDesc.Format;

		txbuff->m_uploaded = true;

		const UINT64 uploadBufferSize
			= txbuff->m_subresouceData.SlicePitch;

		CD3DX12_HEAP_PROPERTIES upHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		ThrowIfFailed(pD3D->CreateCommittedResource(
			&upHeap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&txbuff->m_pTextureUploadHeap)));

	}

	return hres;
}
