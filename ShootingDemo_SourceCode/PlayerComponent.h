#pragma once
#include "GameObject.h"
#include "MySprite.h"

class PlayerComponent : public GameComponent
{
private:
	int m_playerHitpoint;		Å@
	int m_hitDamage;			

	HitBoxClass bodyHit;		
	MySprite* m_pSprite = nullptr;


public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void hitReaction(GameObject* obj, HitBoxClass* hit) override;Å@
};
