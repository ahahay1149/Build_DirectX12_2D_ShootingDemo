#pragma once

#include <list>
#include "GameObject.h"

class GameObjectManager
{
private:
	std::list<GameObject*> m_pObjects;
	GameObjectManager()
	{
		m_pObjects.clear();
	}

public:
	static GameObjectManager* getInstance()
	{
		static GameObjectManager* instance = nullptr;

		if (instance == nullptr)
		{
			instance = new GameObjectManager();
		}

		return instance;
	}

	int addGameObject(GameObject* p_gobj);
	void removeGameObject(GameObject* p_gobj);
	void action();

	void cleanupGameObjectManager();
};


