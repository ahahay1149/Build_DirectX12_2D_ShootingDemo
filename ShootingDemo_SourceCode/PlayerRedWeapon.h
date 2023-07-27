#pragma once
#include "PlayerWeapon.h"

class PlayerRedBulletComponent : public PlayerBulletBase
{
private:
	int		m_level;										
	XMFLOAT2 m_direction;									�@
	int m_bulletPower;			�@
	int m_hitDamage;			�@

	HitBoxClass bulletHit;		
public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;
	void hitReaction(GameObject* obj, HitBoxClass* hit) override;

	void initWeapon(XMFLOAT2 dir, int level) override;		
};
