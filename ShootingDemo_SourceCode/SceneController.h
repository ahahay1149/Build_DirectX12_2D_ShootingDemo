#pragma once

#include <Windows.h>

enum class GAME_SCENES
{
	AWAKE,
	INIT,
	TITLE,
	IN_GAME,
	GAME_OVER,
};

class SceneController
{
protected:
	UINT m_scene;

public:
	HRESULT virtual initSceneController();
	HRESULT virtual changeGameScene(UINT scene);
};