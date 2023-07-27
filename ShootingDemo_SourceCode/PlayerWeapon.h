#pragma once
#include <Windows.h>
#include "GameObject.h"

class PlayerBulletBase : public GameComponent					
{
public:
	virtual void initWeapon(XMFLOAT2 dir, int level) = 0;		
};

class PlayerWeaponComponent : public GameComponent				
{
private:
	BYTE m_weaponColor;											Å@
	int m_weaponLevel;											Å@

	int m_counter;												

public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void setWeapon(BYTE color, int level);						
};