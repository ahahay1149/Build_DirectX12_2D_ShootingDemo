#include "SceneController.h"

#include "MyGameEngine.h"
#include "SoundManager.h"

#include "GameObjectManager.h"
#include "ZakoEnemy.h"
#include "PlayerObject.h"

HRESULT SceneController::initSceneController()
{
	m_scene = static_cast<UINT>(GAME_SCENES::AWAKE);

	return changeGameScene(static_cast<UINT>(GAME_SCENES::INIT));
}

HRESULT SceneController::changeGameScene(UINT scene)
{
	HRESULT hr = S_OK;
	MyGameEngine* engine;

	if (m_scene != scene)
	{
		engine = MyGameEngine::getInstance();

		switch (scene)
		{
		case static_cast<UINT>(GAME_SCENES::INIT):
		{
			SoundManager* soMng = SoundManager::getInstance();

			int soundId = 0;

			hr = engine->loadTextureFile(0, L"./Resources/textures/texture.png");

			if (FAILED(hr))
			{
				return hr;
			}

			hr = engine->loadTextureFile(1, L"./Resources/textures/SSExplosion.png");

			if (FAILED(hr))
			{
				return hr;
			}
			hr = engine->uploadTexture();
			if (FAILED(hr))
			{
				return hr;
			}

			if (!soMng->loadSoundFile(L"./Resources/sounds/playerShot.wav", soundId))
				return E_FAIL;
			if (!soMng->loadSoundFile(L"./Resources/sounds/playerOut.wav", soundId))
				return E_FAIL;
			if (!soMng->loadSoundFile(L"./Resources/sounds/enemyHit.wav", soundId))
				return E_FAIL;
			if (!soMng->loadSoundFile(L"./Resources/sounds/enemyOut.wav", soundId))
				return E_FAIL;
			if (!soMng->loadSoundFile(L"./Resources/sounds/Barrier.wav", soundId))
				return E_FAIL;

			changeGameScene(static_cast<UINT>(GAME_SCENES::IN_GAME));

		}
			break;

		case static_cast<UINT>(GAME_SCENES::TITLE):
			break;

		case static_cast<UINT>(GAME_SCENES::IN_GAME):
		{
			GameObjectManager* goMng = GameObjectManager::getInstance();

			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			goMng->addGameObject(new ZakoEnemy());
			
			goMng->addGameObject(new PlayerObject());
		}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_OVER):
			break;

		default:
			return E_FAIL;	
		}

		m_scene = scene;
		
		engine->WaitForGpu();
	}

	return hr;
}
