#pragma once

#include <list>
#include <memory>
#include "MySprite.h"

#include "HitManager.h"
using namespace std;

class MySprite;

class GameObject;								
class HitBoxClass;								
class GameComponent								
{
private:
	GameObject* m_pGObject;				
	virtual void initAction() = 0;		

public:
	void initFromGameObject(GameObject* objData)
	{
		m_pGObject = objData;
		initAction();
	}

	virtual bool frameAction() = 0;		Å@
	virtual void finishAction() = 0;	

	virtual void hitReaction(GameObject* targetGo, HitBoxClass* hit) {};
	GameObject* getGameObject() { return m_pGObject; }
};

class GameObject								
{
protected:
	std::unique_ptr<MySprite> m_pSprite = nullptr;	
	list<GameComponent*> m_pComponents;			

public:
	GameObject();

	MySprite* getMySprite()
	{
		return m_pSprite.get();
	}

	void addComponent(GameComponent* com)		
	{
		com->initFromGameObject(this);			
		m_pComponents.push_back(com);
	}

	void removeComponent(GameComponent* com)	
	{
		com->finishAction();					
		m_pComponents.remove(com);
		delete(com);							
	}

	virtual void cleanupGameObject();			Å@
	virtual bool action();						 
	virtual void init();						

};
