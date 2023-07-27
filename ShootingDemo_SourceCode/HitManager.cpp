#include <list>

#include "HitManager.h"

bool checkCornerCircle(const XMFLOAT2* cornerPos, const XMFLOAT2* hitCenter, float radius)
{
	float xdist = hitCenter->x - cornerPos->x;
	float ydist = hitCenter->y - cornerPos->y;

	if ((xdist * xdist + ydist * ydist) > (radius * radius))
	{
		return false;
	}

	return true;
};

bool isHitRectVSCircle(const XMFLOAT4* rectHit, const XMFLOAT2* circleCenter, const float radius)
{
	XMFLOAT2 cornerPos;

	cornerPos.x = rectHit->x;
	cornerPos.y = rectHit->y;
	if (checkCornerCircle(&cornerPos, circleCenter, radius))
		return true;
	cornerPos.y = rectHit->w;
	if (checkCornerCircle(&cornerPos, circleCenter, radius))
		return true;

	cornerPos.x = rectHit->z;
	cornerPos.y = rectHit->y;
	if (checkCornerCircle(&cornerPos, circleCenter, radius))
		return true;
	cornerPos.y = rectHit->w;
	if (checkCornerCircle(&cornerPos, circleCenter, radius))
		return true;

	float x_min = rectHit->x;
	float x_max = rectHit->z;
	float y_min = rectHit->y;
	float y_max = rectHit->w;

	if ((x_min - radius) > circleCenter->x || (x_max + radius) < circleCenter->x
		|| y_min > circleCenter->y || y_max < circleCenter->y)
	{
		return false;
	}

	if (x_min > circleCenter->x || x_max < circleCenter->x
		|| (y_min - radius) > circleCenter->y || (y_max + radius) < circleCenter->y)
	{
		return false;
	}

	return true;
}

void HitManager::flushHitList(std::list<HitBoxStructure*>* p_hitlist)
{
	for (auto hitStr : *p_hitlist)
	{
		delete(hitStr);
	}

	p_hitlist->clear();
}

HitManager::HitManager()
{
	playerBodyHit.clear();		
	enemyBodyHit.clear();		
	playerWeaponHit.clear();	
	enemyWeaponHit.clear();		
}

void HitManager::refreshHitSystem()	
{
	flushHitList(&playerBodyHit);
	flushHitList(&playerWeaponHit);
	flushHitList(&enemyBodyHit);
	flushHitList(&enemyWeaponHit);
}

void HitManager::setBoxHit(GameComponent* cmp, HitBoxClass* box)
{
	HitBoxStructure* hit = new HitBoxStructure(cmp, box);	
	switch (box->getHitType())
	{
	case HIT_AREA_TYPE::HIT_PLAYER_BODY:
		playerBodyHit.push_back(hit);					
		break;

	case HIT_AREA_TYPE::HIT_PLAYER_ATTACK:
		playerWeaponHit.push_back(hit);					
		break;

	case HIT_AREA_TYPE::HIT_ENEMY_BODY:
		enemyBodyHit.push_back(hit);					
		break;

	case HIT_AREA_TYPE::HIT_ENEMY_ATTACK:
		enemyWeaponHit.push_back(hit);				@
		break;
	case HIT_AREA_TYPE::HIT_PLAYER_SHIELD:
		break;

	case HIT_AREA_TYPE::HIT_ENEMY_SHIELD:
		break;
	}
}

bool HitManager::HitBoxStructure::isHit(HitBoxStructure* target)	
{
	XMFLOAT4 myRect = m_pHitbox->getHitRect();					
	XMFLOAT4 targetRect = target->m_pHitbox->getHitRect();		

	if (targetRect.x > myRect.z || targetRect.z < myRect.x)
	{
		return false;	
	}

	if (targetRect.y > myRect.w || targetRect.w < myRect.y)
	{
		return false;	
	}

	return true;		
}

bool HitManager::HitBoxStructure::isHit(HitCircleStructure* target)
{
	XMFLOAT4 hitRect = m_pHitbox->getHitRect();
	XMFLOAT2 centerPos = target->getHitCircle()->getCenterPosition();
	return isHitRectVSCircle(&hitRect, &centerPos, target->getHitCircle()->getRadius());
}

bool HitManager::HitCircleStructure::isHit(HitBoxStructure* target)
{
	XMFLOAT4 hitrect = target->getHitBox()->getHitRect();
	XMFLOAT2 centerPos = m_pHitCircle->getCenterPosition();
	return isHitRectVSCircle(&hitrect, &centerPos, m_pHitCircle->getRadius());
}

bool HitManager::HitCircleStructure::isHit(HitCircleStructure* target)
{
	HitCircleClass* tgtCircle = target->getHitCircle();
	XMFLOAT2 centerA = m_pHitCircle->getCenterPosition();
	XMFLOAT2 centerB = tgtCircle->getCenterPosition();

	float xdist = centerB.x - centerA.x;	
	float ydist = centerB.y - centerA.y;	

	float rr = m_pHitCircle->getRadius() + tgtCircle->getRadius();
	if ( (rr * rr) < (xdist * xdist + ydist * ydist))
	{
		return false;
	}

	return true;				
}

GameComponent* HitManager::HitCircleStructure::getGameComponent()	
{
	return m_pGameComponent;
}

HitCircleClass* HitManager::HitCircleStructure::getHitCircle()		
{
	return m_pHitCircle;
}
GameComponent* HitManager::HitBoxStructure::getGameComponent()	
{
	return m_pGameComponent;
}

HitBoxClass* HitManager::HitBoxStructure::getHitBox()		
{
	return m_pHitbox;
}

void HitManager::hitFrameAction()
{
	for (auto p_enHit : enemyWeaponHit)
	{
		for (auto p_plHit : playerBodyHit)
		{
			if (p_enHit->isHit(p_plHit))
			{
				p_enHit->getGameComponent()->hitReaction(p_plHit->getGameComponent()->getGameObject(), p_plHit->getHitBox());
				p_plHit->getGameComponent()->hitReaction(p_enHit->getGameComponent()->getGameObject(), p_enHit->getHitBox());
			}
		}
	}

	for (auto p_plHit : playerWeaponHit)
	{
		for (auto p_enHit : enemyBodyHit)
		{
			if (p_plHit->isHit(p_enHit))
			{
				p_enHit->getGameComponent()->hitReaction(p_plHit->getGameComponent()->getGameObject(), p_plHit->getHitBox());
				p_plHit->getGameComponent()->hitReaction(p_enHit->getGameComponent()->getGameObject(), p_enHit->getHitBox());
			}
		}
	}

}

void HitBoxClass::updateHitRect()						 
{
	float hw = m_hitSize.x * 0.5f;			
	float hh = m_hitSize.y * 0.5f;			

	m_hitRect.x = m_hitCenter.x - hw;		
	m_hitRect.y = m_hitCenter.y - hh;		
	m_hitRect.z = m_hitCenter.x + hw;		
	m_hitRect.w = m_hitCenter.y + hh;		
}

