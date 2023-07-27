#pragma once
#include <Windows.h>
#include <d3d12.h>

#include <vector>

#include <memory>
#include <wrl/client.h>

#define MAX_TEXTURES (20)

using Microsoft::WRL::ComPtr;

struct Texture2DContainer
{
	ComPtr<ID3D12Resource> m_pTexture;
	ComPtr<ID3D12Resource> m_pTextureUploadHeap;

	bool m_uploaded = false;
	bool m_viewCreated = false;

	std::unique_ptr<uint8_t[]> m_wicData;
	D3D12_SUBRESOURCE_DATA m_subresouceData;

	float fWidth;
	float fHeight;
	DXGI_FORMAT texFormat;
};

class TextureManager
{
private:
	Texture2DContainer m_textureArray[MAX_TEXTURES];

	HRESULT createTextureFromFile(ID3D12Device* g_pD3D, Texture2DContainer* txbuff, const wchar_t* filename, bool genMipmap = false);
	void	releaseTexObj(Texture2DContainer* txbuff);
	void	destructTextureManager(void);

public:

	HRESULT initTextureManager(void);

	HRESULT createTextureFromFile(ID3D12Device* pD3D, int index, const wchar_t* filename);

	HRESULT uploadCreatedTextures(ID3D12Device* pD3D, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandQueue* pCmdQueue);

	void releaseTexture(int index);
	void releaseAllTextures(void);

	void setTexture(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, UINT texIndex);
	void setTexture(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, Texture2DContainer* txbuff);

	void unsetTexture(ID3D12CommandList* pCommandList);

	Texture2DContainer* getTexture(int index);

	~TextureManager();
};
