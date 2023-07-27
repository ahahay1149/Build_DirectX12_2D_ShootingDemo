#include "GameObject.h"

GameObject::GameObject()
{
	m_pSprite.reset(new MySprite);
}

void GameObject::init()
{
}

bool GameObject::action()	
{
	list<GameComponent*> deleteComponents;	
	deleteComponents.clear();				

	for (auto com : m_pComponents)	 
	{
		if (!com->frameAction())							
		{
			deleteComponents.push_back(com);
		}
	}

	if (!deleteComponents.empty())
	{
		for (auto com : deleteComponents)	 
		{
			removeComponent(com);		
		}

		deleteComponents.clear();		

		if (m_pComponents.empty())			
		{
			return false;				
		}
	}

	return true;						
}

void GameObject::cleanupGameObject()							
{
	for (auto com : m_pComponents)	 
	{
		com->finishAction();	
		delete(com);			
	}

	m_pComponents.clear();											
}