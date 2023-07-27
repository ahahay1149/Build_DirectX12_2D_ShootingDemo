#include "MyGameEngine.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "PlayerComponent.h"

#include "EffectAnimeObject.h"
#include "GameObjectManager.h"
#define PLAYER_MOVE_SPEED (2.0f)
#define PLAYER_45_SPEED (PLAYER_MOVE_SPEED / (1.41421356f))

void PlayerComponent::initAction()
{

	MySprite* sprite = getGameObject()->getMySprite();
	sprite->setTexId(0);
	sprite->setSprite(XMINT4{ 0, 0, 20, 24 });
	sprite->setSize(20, 24);

	m_playerHitpoint = 1;
	m_hitDamage = 0;

	bodyHit.setAttackType(HIT_AREA_TYPE::HIT_PLAYER_BODY, m_playerHitpoint);	
	bodyHit.setHitSize(10.0f, 12.0f);			Å@	
}

void PlayerComponent::finishAction()
{
}

enum DIRECTION_CODE
{
	MOVE_UP = 0x00000001,
	MOVE_DOWN = 0x00000002,
	MOVE_LEFT = 0x00000004,
	MOVE_RIGHT = 0x00000008,

	MOVE_UPLEFT = 0x00000005,
	MOVE_UPRIGHT = 0x00000009,
	MOVE_DOWNLEFT = 0x00000006,
	MOVE_DOWNRIGHT = 0x0000000a,
};

bool PlayerComponent::frameAction()
{
	MyGameEngine* pEngine = MyGameEngine::getInstance();
	InputManager* pInput = InputManager::getInstance();
	HitManager* pHit = HitManager::getInstance();
	MySprite* sprite = getGameObject()->getMySprite();

	byte pressFlg = 0;							Å@

	m_playerHitpoint -= m_hitDamage;	
	m_hitDamage = 0;					

	if (m_playerHitpoint < 1)			
	{
		SoundManager::getInstance()->play(1);

		XMFLOAT3 pos = *sprite->getPosition();
		GameObjectManager::getInstance()->addGameObject(new PlayerCrashEffect(pos));
		return false;
	}

	if (pInput->checkKeyboard(VK_UP, InputManager::BUTTON_STATE::BUTTON_DOWN))				
	{
		pressFlg = DIRECTION_CODE::MOVE_UP;
	}
	else if (pInput->checkKeyboard(VK_DOWN, InputManager::BUTTON_STATE::BUTTON_DOWN))
	{
		pressFlg = DIRECTION_CODE::MOVE_DOWN;
	}

	if (pInput->checkKeyboard(VK_LEFT, InputManager::BUTTON_STATE::BUTTON_DOWN))
	{
		pressFlg |= DIRECTION_CODE::MOVE_LEFT;
	}
	else if (pInput->checkKeyboard(VK_RIGHT, InputManager::BUTTON_STATE::BUTTON_DOWN))
	{
		pressFlg |= DIRECTION_CODE::MOVE_RIGHT;
	}

	XMFLOAT3 pos = *sprite->getPosition();		

	switch (pressFlg)
	{
	case DIRECTION_CODE::MOVE_UP:
		pos.y += PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWN:
		pos.y -= PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_LEFT:
		pos.x -= PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_RIGHT:
		pos.x += PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_UPLEFT:			
		pos.x -= PLAYER_45_SPEED;
		pos.y += PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_UPRIGHT:
		pos.x += PLAYER_45_SPEED;
		pos.y += PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWNLEFT:
		pos.x -= PLAYER_45_SPEED;
		pos.y -= PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWNRIGHT:
		pos.x += PLAYER_45_SPEED;
		pos.y -= PLAYER_45_SPEED;
		break;
	}

	sprite->setPosition(pos.x, pos.y, pos.z);

	bodyHit.setAttackType(HIT_AREA_TYPE::HIT_PLAYER_BODY, m_playerHitpoint);	
	bodyHit.setPosition(pos.x, pos.y);											

	pHit->setBoxHit(this, &bodyHit);	

	MyGameEngine::getInstance()->getSpriteRenderer()->addMySprite(sprite);

	return true;
}

void PlayerComponent::hitReaction(GameObject* obj, HitBoxClass* hit)
{
	switch (hit->getHitType())
	{
	case HIT_AREA_TYPE::HIT_ENEMY_ATTACK:	
		m_hitDamage += hit->getHitPower();	
		break;

	default:	
		break;
	}
}
