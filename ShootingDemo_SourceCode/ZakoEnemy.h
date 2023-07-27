#pragma once
#include "GameObject.h"

#include <random>

class ZakoEnemyComponent : public GameComponent
{
private:
	int m_enemyHitpoint = 0;		Å@
	int m_hitDamage = 0;			
	std::random_device m_rndDev;	

	HitBoxClass bodyHit = {};		
	HitBoxClass bodyAttackHit = {};	

public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;
	void hitReaction(GameObject* targetGo, HitBoxClass* hit) override;
};

class ZakoEnemy : public GameObject
{
private:

public:
	ZakoEnemy()
	{
		addComponent(new ZakoEnemyComponent());
	}

};
