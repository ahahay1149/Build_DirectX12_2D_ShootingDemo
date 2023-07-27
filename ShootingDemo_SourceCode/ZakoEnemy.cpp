#include "MyGameEngine.h"
#include "HitManager.h"
#include "SoundManager.h"

#include "ZakoEnemy.h"

#include "EffectAnimeObject.h"
#include "GameObjectManager.h"
void ZakoEnemyComponent::hitReaction(GameObject* obj, HitBoxClass* hit)
{
	switch (hit->getHitType())
	{
	case HIT_AREA_TYPE::HIT_PLAYER_BODY:
		m_hitDamage += hit->getHitPower();
		break;

	case HIT_AREA_TYPE::HIT_PLAYER_ATTACK:
		m_hitDamage += hit->getHitPower();
		break;

	default:
		break;
	}
}

void ZakoEnemyComponent::initAction()
{
	m_enemyHitpoint = 10;	
	m_hitDamage = 0;

	MySprite* sprite = getGameObject()->getMySprite();

	sprite->setTexId(0);
	sprite->setSprite(XMINT4(0, 25, 14, 14));
	sprite->setSize(14, 14);

	XMFLOAT3 initpos{ 0.0f, 0.0f, 0.2f };
	initpos.x = 120.0f - 240.0f * (float)(m_rndDev() % 100) / 100.0f;
	initpos.y = 135.0f + 100.0f * (float)(m_rndDev() % 100) / 100.0f;

	sprite->setPosition(initpos.x, initpos.y, initpos.z);

	bodyHit.setAttackType(HIT_AREA_TYPE::HIT_ENEMY_BODY, m_enemyHitpoint);
	bodyHit.setHitSize(10.0f, 10.0f);						

	bodyAttackHit.setAttackType(HIT_AREA_TYPE::HIT_ENEMY_ATTACK, m_enemyHitpoint);
	bodyAttackHit.setHitSize(7.0f, 7.0f);			
}

bool ZakoEnemyComponent::frameAction()
{
	MySprite* sprite = getGameObject()->getMySprite();
	XMFLOAT3 pos = *sprite->getPosition();

	pos.y -= 1.5f;

	if (pos.y < -135.0f)
	{
		initAction();
	}
	else
	{
		sprite->setPosition(pos.x, pos.y, pos.z);
	}

	m_enemyHitpoint -= m_hitDamage;

	if (m_enemyHitpoint < 1)
	{
		SoundManager::getInstance()->play(3);

		GameObjectManager::getInstance()->addGameObject(new ZakoEnemyCrashEffect(pos));
		initAction();			
		return true;
	}
	else if (m_hitDamage > 0)
	{
		SoundManager::getInstance()->play(2);
	}

	m_hitDamage = 0;

	bodyHit.setPosition(pos.x, pos.y);
	bodyAttackHit.setPosition(pos.x, pos.y);

	HitManager* hitMng = HitManager::getInstance();

	hitMng->setBoxHit(this, &bodyHit);
	hitMng->setBoxHit(this, &bodyAttackHit);

	MyGameEngine::getInstance()->getSpriteRenderer()->addMySprite(sprite);
	return true;
}

void ZakoEnemyComponent::finishAction()
{
}
