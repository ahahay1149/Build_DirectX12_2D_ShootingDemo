#include "PlayerRedWeapon.h"

#include "MyGameEngine.h"
#include "HitManager.h"

#include "GameObjectManager.h"
#include "EffectAnimeObject.h"
void PlayerRedBulletComponent::initAction()
{
	m_bulletPower = 2;
	m_hitDamage = 0;

	bulletHit.setAttackType(HIT_AREA_TYPE::HIT_PLAYER_ATTACK, m_bulletPower);
	bulletHit.setHitSize(4.0f, 12.0f);	

	MySprite* sprite = getGameObject()->getMySprite();
	sprite->setTexId(0);

	switch (m_level)
	{
	default:
		sprite->setSprite(XMINT4{ 21, 0, 3, 12 });	       
		sprite->setSize(3, 12);
		break;
	}
}

void PlayerRedBulletComponent::initWeapon(XMFLOAT2 dir, int level)
{
	m_direction.x = dir.x;		
	m_direction.y = dir.y;

	m_level = level;
}

void PlayerRedBulletComponent::finishAction()
{

}

bool PlayerRedBulletComponent::frameAction()
{
	MySprite* sprite = getGameObject()->getMySprite();

	XMFLOAT3 pos = *sprite->getPosition();

	pos.x += m_direction.x;
	pos.y += m_direction.y;
	sprite->setPosition(pos.x, pos.y, pos.z);

	if (pos.y > 140.0f)
	{
		return false;
	}

	if (m_hitDamage > 0)
	{
		XMFLOAT3 effectPos = pos;
		effectPos.z -= 0.2f;	
		ZakoEnemyCrashEffect* efx = new ZakoEnemyCrashEffect(effectPos);
		efx->getMySprite()->setSize(8, 8);
		GameObjectManager::getInstance()->addGameObject(efx);
	}
	m_bulletPower -= m_hitDamage;		
	m_hitDamage = 0;					

	if (m_bulletPower < 1)				
	{
		return false;
	}

	bulletHit.setAttackType(HIT_AREA_TYPE::HIT_PLAYER_ATTACK, m_bulletPower);		
	bulletHit.setPosition(pos.x, pos.y);											

	HitManager::getInstance()->setBoxHit(this, &bulletHit);	

	MyGameEngine::getInstance()->getSpriteRenderer()->addMySprite(sprite);
	return true;
}

void PlayerRedBulletComponent::hitReaction(GameObject* obj, HitBoxClass* hit)
{
	switch (hit->getHitType())
	{
	case HIT_AREA_TYPE::HIT_ENEMY_BODY:
		m_hitDamage += hit->getHitPower();	
		break;

	default:								
		break;
	}
}
