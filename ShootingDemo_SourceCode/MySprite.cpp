#include "MySprite.h"
#include "MyGameEngine.h"

MySprite::MySprite()
{
	m_uv.x = 0.0f;
	m_uv.y = 0.0f;
	m_uv.z = 1.0f;
	m_uv.w = 1.0f;

	m_size.x = 1.0f;
	m_size.y = 1.0f;

	setPosition(0, 0, 0);		
	setRotationRad(0, 0, 0);	
	setScale(1, 1, 1);			
	m_color = { 1, 1, 1, 1 };
	m_colorMix = (UINT)COLOR_MIX_OP::MIX_MUL;
	m_alphaMix = (UINT)COLOR_MIX_OP::MIX_MUL;
}

void MySprite::setSprite(XMINT4 sprite)
{
	m_uv.x = (float)sprite.x / m_textureSize.x;
	m_uv.y = (float)sprite.y / m_textureSize.y;
	m_uv.z = (float)sprite.z / m_textureSize.x;
	m_uv.w = (float)sprite.w / m_textureSize.y;
}

void MySprite::setUV(XMFLOAT4 uv)
{
	m_uv = uv;
}

void MySprite::setSize(float width, float height)
{
	m_size.x = width;
	m_size.y = height;
}

const XMFLOAT4* MySprite::getUV()
{
	return &m_uv;
}

const XMFLOAT2* MySprite::getSize()
{
	return &m_size;
}

void MySprite::setPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void MySprite::setScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
}

void MySprite::setRotationDeg(float x, float y, float z)
{
	m_rotation.x = XMConvertToRadians(x);
	m_rotation.y = XMConvertToRadians(y);
	m_rotation.z = XMConvertToRadians(z);
}

void MySprite::setRotationRad(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

const XMFLOAT3* MySprite::getPosition()
{
	return &m_position;
}

const XMFLOAT3* MySprite::getScale()
{
	return &m_scale;
}

const XMFLOAT3* MySprite::getRotation()
{
	return &m_rotation;
}

void MySprite::setColor(float r, float g, float b, float a)
{
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
}

void MySprite::setColorMix(COLOR_MIX_OP op)
{
	m_colorMix = (UINT)op;
}

void MySprite::setAlphaMix(COLOR_MIX_OP op)
{
	m_alphaMix = (UINT)op;
}

const XMFLOAT4* MySprite::getColor()
{
	return &m_color;
}

UINT MySprite::getColorMix()
{
	return m_colorMix;
}

UINT MySprite::getAlphaMix()
{
	return m_alphaMix;
}
void MySprite::setTexId(int texId)
{
	Texture2DContainer* texture = MyGameEngine::getInstance()->getTextureManager()->getTexture(texId);

	if (texture != nullptr)
	{
		m_texId = texId;
		m_textureSize.x = texture->fWidth;
		m_textureSize.y = texture->fHeight;
	}
}
