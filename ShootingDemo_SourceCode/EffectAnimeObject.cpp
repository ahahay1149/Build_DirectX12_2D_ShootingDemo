#include "MyGameEngine.h"
#include "EffectAnimeObject.h"

void EffectAnimeComponent::setSpriteUV()
{
	int u = m_uvStart.x + (m_anime % m_maxXCount) * m_uvSize.x;
	int v = m_uvStart.y + (m_anime / m_maxXCount) * m_uvSize.y;

	MySprite* sp = getGameObject()->getMySprite();
	sp->setSprite(XMINT4(u, v, m_uvSize.x, m_uvSize.y));
}

EffectAnimeComponent::EffectAnimeComponent()
{
	m_uvStart = {};
	m_uvSize = {};

	m_waitFrames = 0;
	m_animePatters = 0;

	m_waitCount = 0;
	m_anime = 0;

	m_maxXCount = 0;
}

void EffectAnimeComponent::setAnimeParameter(UINT16 uOff, UINT16 vOff, UINT16 uSize, UINT16 vSize, UINT16 waitFrames, UINT16 animePatterns)
{
	MySprite* sp = getGameObject()->getMySprite();

	m_maxXCount = MyGameEngine::getInstance()->getTextureManager()->getTexture(sp->getTexId())->fWidth / uSize;

	m_uvStart = { uOff, vOff };
	m_uvSize = { uSize, vSize };

	m_waitFrames = waitFrames;
	m_animePatters = animePatterns;

	setSpriteUV();	
}

void EffectAnimeComponent::initAction()
{
}

bool EffectAnimeComponent::frameAction()
{
	m_waitCount++;
	
  	if (m_waitCount >= m_waitFrames)
	{
		m_waitCount = 0;
		m_anime++;

		if (m_anime >= m_animePatters)
		{
			return false;	
		}

		setSpriteUV();
	}

	MyGameEngine::getInstance()->getSpriteRenderer()->addMySprite(getGameObject()->getMySprite());

    return true;
}

void EffectAnimeComponent::finishAction()
{
}

EffectAnimeObject::EffectAnimeObject(XMFLOAT3& position, UINT16 textureId)
{
    m_pSprite->setTexId(textureId);
    m_pSprite->setPosition(position.x, position.y, position.z);
}

void EffectAnimeObject::setAnimeParameter(UINT16 uOff, UINT16 vOff, UINT16 uSize, UINT16 vSize, UINT16 waitFrames, UINT16 animePatterns)
{
	EffectAnimeComponent* efxComp = new EffectAnimeComponent();
	addComponent(efxComp);

	efxComp->setAnimeParameter(uOff, vOff, uSize, vSize, waitFrames, animePatterns);
}
