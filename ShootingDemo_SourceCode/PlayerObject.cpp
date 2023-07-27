#include "PlayerObject.h"
#include "PlayerComponent.h"
#include "PlayerWeapon.h"
void PlayerObject::init()
{
	addComponent(new PlayerComponent());			
	addComponent(new PlayerWeaponComponent());		
}
