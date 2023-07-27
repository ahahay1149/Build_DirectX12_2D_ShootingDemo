#pragma once

#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class MySprite
{
private:
	int m_texId;
	XMFLOAT4 m_uv;
	XMFLOAT2 m_size;

	XMFLOAT2 m_textureSize;

	XMFLOAT3 m_position;	
	XMFLOAT3 m_scale;		
	XMFLOAT3 m_rotation;	
	XMFLOAT4 m_color;
	UINT	m_colorMix;
	UINT	m_alphaMix;

public:
	enum class COLOR_MIX_OP
	{
		MIX_MUL = 0,
		MIX_ADD = 1,
		MIX_DEC = 2,
		MIX_SET = 3,
		MIX_NONE = 4,
	};
	MySprite();	

	void setSprite(XMINT4 sprite);
	void setUV(XMFLOAT4 uv);
	void setSize(float width, float height);

	const XMFLOAT4* getUV();
	const XMFLOAT2* getSize();

	void setPosition(float x, float y, float z);
	void setScale(float x, float y, float z);
	void setRotationDeg(float x, float y, float z);
	void setRotationRad(float x, float y, float z);

	const XMFLOAT3* getPosition();	
	const XMFLOAT3* getScale();		
	const XMFLOAT3* getRotation();	
	void setColor(float r, float g, float b, float a);
	void setColorMix(COLOR_MIX_OP op);	
	void setAlphaMix(COLOR_MIX_OP op);

	const XMFLOAT4* getColor();
	UINT	getColorMix();
	UINT	getAlphaMix();
	void setTexId(int texId);
	int getTexId() { return m_texId; }
};

