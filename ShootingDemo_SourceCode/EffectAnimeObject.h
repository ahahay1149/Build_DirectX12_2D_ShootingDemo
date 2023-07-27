#pragma once
#include "GameObject.h"

class EffectAnimeComponent : 
    public GameComponent
{
private:
	XMINT2 m_uvStart;
	XMINT2 m_uvSize;

	int		m_maxXCount;

	int		m_waitFrames;
	int		m_animePatters;

	int		m_waitCount;
	int		m_anime;

	void setSpriteUV();

public:
	EffectAnimeComponent();

	void setAnimeParameter(UINT16 uOff, UINT16 vOff, UINT16 uSize, UINT16 vSize, UINT16 waitFrames, UINT16 animePatterns);

	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

};

class EffectAnimeObject :
    public GameObject
{
public:
	EffectAnimeObject(XMFLOAT3& position, UINT16 textureId);
	void setAnimeParameter(UINT16 uOff, UINT16 vOff, UINT16 uSize, UINT16 vSize, UINT16 waitFrames, UINT16 animePatterns);
};

class ZakoEnemyCrashEffect :
	public EffectAnimeObject
{
public:
	ZakoEnemyCrashEffect(XMFLOAT3& position) : EffectAnimeObject(position, 1)
	{
		setAnimeParameter(0, 0, 32, 32, 3, 4);
		m_pSprite->setSize(16, 16);
	}
};

class PlayerCrashEffect :
	public EffectAnimeObject
{
public:
	PlayerCrashEffect(XMFLOAT3& position) : EffectAnimeObject(position, 1)
	{
		setAnimeParameter(0, 32, 64, 48, 5, 4);
		m_pSprite->setSize(32, 24);
	}
};


