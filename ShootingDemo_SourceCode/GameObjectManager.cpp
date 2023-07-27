#include "GameObjectManager.h"

int GameObjectManager::addGameObject(GameObject* p_gobj)
{
    p_gobj->init();
    m_pObjects.push_back(p_gobj);
    return m_pObjects.size();
}

void GameObjectManager::removeGameObject(GameObject* p_gobj)
{
    m_pObjects.remove(p_gobj);
    p_gobj->cleanupGameObject();
}

void GameObjectManager::action()
{
    list<GameObject*> deleteObjects;    
    deleteObjects.clear();              

    HitManager* hitMng = HitManager::getInstance();
    hitMng->refreshHitSystem();
    for (auto gameObj : m_pObjects)
    {
        if (!gameObj->action())
            deleteObjects.push_back(gameObj);
    }

    hitMng->hitFrameAction();
    for (auto gameObj : deleteObjects)
    {
        gameObj->cleanupGameObject();
        m_pObjects.remove(gameObj);
        delete(gameObj);
    }

    deleteObjects.clear();
}

void GameObjectManager::cleanupGameObjectManager()
{
    for (auto go : m_pObjects)
    {
        go->cleanupGameObject();
    }

    m_pObjects.clear();
}
