#include "MyGameEngine.h"	
#include "PlayerWeapon.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "GameObjectManager.h"

#include "PlayerRedWeapon.h"

void PlayerWeaponComponent::initAction()	
{
	m_weaponColor = 0;	
	m_weaponLevel = 0;	
	m_counter = 0;		
}

bool PlayerWeaponComponent::frameAction()
{
	GameObject* pObj = getGameObject();
	MySprite* sprite = pObj->getMySprite();

	MyGameEngine* pEngine = MyGameEngine::getInstance();
	InputManager* inputMng = InputManager::getInstance();
	HitManager* hitMng = HitManager::getInstance();

	XMFLOAT3 pos;					
	GameObject* bltObj;				
	PlayerRedBulletComponent* bltCmp;	
	XMFLOAT2 direction;				
	float f_temp;					

	if (inputMng->checkKeyboard(VK_SPACE, InputManager::BUTTON_STATE::BUTTON_DOWN))
	{
		switch (m_weaponColor)
		{
		case 0:		
			if (m_counter == 0)
			{
				pos = *sprite->getPosition();

				GameObjectManager* goMng = GameObjectManager::getInstance();

				switch (m_weaponLevel)
				{
				default:	

					f_temp = pos.x;							

					direction.x = 0.0f;						
					direction.y = 6.0f;

					pos.y += 3.0f;	　
					pos.z += 0.01f; 　

					bltObj = new GameObject();												
					bltObj->getMySprite()->setPosition(f_temp - 7.0f, pos.y, pos.z);	
					goMng->addGameObject(bltObj);					

					bltCmp = new PlayerRedBulletComponent();		
					bltCmp->initWeapon(direction, 0);										　
					bltObj->addComponent(bltCmp);											

					bltObj = new GameObject();												
					bltObj->getMySprite()->setPosition(f_temp + 7.0f, pos.y, pos.z);	
					goMng->addGameObject(bltObj);					

					bltCmp = new PlayerRedBulletComponent();		
					bltCmp->initWeapon(direction, 0);										　
					bltObj->addComponent(bltCmp);											

					SoundManager::getInstance()->play(0);
					break;
				}
			}
			m_counter = (m_counter + 1) % 3;	
			break;

		case 1:		
			break;

		case 2:		
			break;

		default:	
			break;

		}
	}
	else
	{
		m_counter = 0;
	}

	return true;
}

void PlayerWeaponComponent::finishAction()
{

}

void PlayerWeaponComponent::setWeapon(BYTE color, int level)
{
	m_weaponColor = color;	
	m_weaponLevel = level;	
}
