#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <list>

#include "MyGameEngine.h"
#include "MySprite.h"

using Microsoft::WRL::ComPtr;

class MyGameEngine;

class MySpriteRenderer
{
private:
	static const UINT MAX_SPRITES = 512;	
	struct MixSettings
	{
		UINT colorMix;
		UINT alphaMix;
	};

	ComPtr<ID3D12Resource> m_pSpriteVertexBuffer = nullptr;		
	ComPtr<ID3D12Resource> m_pSpriteIndexBuffer = nullptr;        

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;


	ComPtr<ID3D12Resource> m_pUvBuffer = nullptr;					
	ComPtr<ID3D12Resource> m_pSizeBuffer = nullptr;				
	
	ComPtr<ID3D12Resource> m_pAffineMatrix = nullptr;				
	ComPtr<ID3D12Resource> m_pColorBuffer = nullptr;				
	ComPtr<ID3D12Resource> m_pMixBuffer = nullptr;				
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;      

	ComPtr<ID3D12GraphicsCommandList> m_commandLists[FRAME_COUNT];

	XMFLOAT4 m_aUv[MAX_SPRITES];
	XMFLOAT2 m_aSize[MAX_SPRITES];
	XMMATRIX m_aAffin[MAX_SPRITES];
	XMFLOAT4 m_aColor[MAX_SPRITES];
	MixSettings m_aMix[MAX_SPRITES];
	std::list<MySprite*> m_spriteList;
	void destructMySpriteRenderer();
public:
	HRESULT initMySpriteRenderer(ID3D12Device2 * d3dDevice);	

	void addMySprite(MySprite* mysp);

	ID3D12CommandList* RenderSprites(MyGameEngine* myEngine);	

	~MySpriteRenderer();
};

