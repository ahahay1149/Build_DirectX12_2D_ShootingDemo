#include "InputManager.h"
#include <memory>

HWND					g_hWnd;							 

#if DIRECT_INPUT_ACTIVE
LPDIRECTINPUT8			g_pDInput = nullptr;			 
LPDIRECTINPUTDEVICE8	g_pDIGamePad[MAX_DI_PADS];		 

#if DI_KEY_MOUSE
LPDIRECTINPUTDEVICE8	g_pDIKeyboard;					 
LPDIRECTINPUTDEVICE8	g_pDIMouse;						 
#endif

UINT					g_DICount = 0;					 

BOOL CALLBACK EnumDIJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	hr = g_pDInput->CreateDevice(pdidInstance->guidInstance, &g_pDIGamePad[g_DICount], NULL);
	if (FAILED(hr)) return DIENUM_CONTINUE;	

	hr = g_pDIGamePad[g_DICount]->SetDataFormat(&c_dfDIJoystick2);	
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	DIPROPDWORD diProp;								
	ZeroMemory(&diProp, sizeof(DIPROPDWORD));
	diProp.diph.dwSize = sizeof(DIPROPDWORD);
	diProp.diph.dwHeaderSize = sizeof(diProp.diph);
	diProp.diph.dwHow = DIPH_DEVICE;
	diProp.diph.dwObj = 0;
	diProp.dwData = DIPROPAXISMODE_ABS;	 
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_AXISMODE, &diProp.diph);

	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	DIPROPRANGE diPrg;
	ZeroMemory(&diPrg, sizeof(DIPROPRANGE));
	diPrg.diph.dwSize = sizeof(DIPROPRANGE);
	diPrg.diph.dwHeaderSize = sizeof(diPrg.diph);
	diPrg.diph.dwHow = DIPH_BYOFFSET;
	diPrg.diph.dwObj = DIJOFS_X;
	diPrg.lMin = -32767;				
	diPrg.lMax = 32767;
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}
	diPrg.diph.dwObj = DIJOFS_Y;
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	hr = g_pDIGamePad[g_DICount]->SetCooperativeLevel(g_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	hr = g_pDIGamePad[g_DICount]->Acquire();
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	goto SUCCESS_POINT;		

FAILED_POINT:

	g_pDIGamePad[g_DICount]->Release();
	g_pDIGamePad[g_DICount] = nullptr;
	return DIENUM_CONTINUE;

SUCCESS_POINT:

	g_DICount++;
	if (g_DICount < MAX_DI_PADS)
		return DIENUM_CONTINUE;

	return DIENUM_STOP;	
}
#endif

InputManager::InputManager()
{
	int i, j;
	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] = 0;
	}
	
	m_wheelDelta = 0;

#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] = 0;
	}
#endif

	for (i = 0; i < MAX_PADS; i++)
	{
		m_xPadActive[i] = false;
		
		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j].x = 0;
			m_analogState[i][j].y = 0;
		}
	}

#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		m_diDirection[i][0] = 0;
		m_diDirection[i][1] = 0;
		m_diDirection[i][2] = 0;
		m_diDirection[i][3] = 0;

		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_diAnalogState[i][j].x = 0;
			m_diAnalogState[i][j].y = 0;
			m_diAnalogState[i][j].z = 0;
		}
	}

#endif

}

InputManager* InputManager::getInstance()
{
	static std::unique_ptr<InputManager> instance = nullptr;

	if (instance == nullptr)
	{
		instance.reset(new InputManager());
	}

	return instance.get();
}

InputManager::~InputManager()
{
	setInputEnable(false);

#if DIRECT_INPUT_ACTIVE
	releaseDirectInput();
#endif
}

HRESULT InputManager::initInputManager(HINSTANCE hInst, HWND hwnd)
{
	g_hWnd = hwnd;		
	refreshBuffer();	
#if DIRECT_INPUT_ACTIVE
	return initDirectInput(hInst);
#else
	return S_OK;
#endif
}

void InputManager::update()
{
	int i;	

#if !DI_KEY_MOUSE
	BYTE currentKeys[256];

	if (GetKeyboardState(currentKeys))
	{
		for (i = 0; i < MAX_KEYS; i++)
		{
			m_keyState[i] <<= 1;	

			if (currentKeys[i] & 0x80)
			{
				m_keyState[i] |= 0x01;
			}
		}

	}

	POINT mousepos;
	GetCursorPos(&mousepos);
	ScreenToClient(g_hWnd, &mousepos);

	m_mouseVect.x = mousepos.x - m_mousePos.x;
	m_mouseVect.y = mousepos.y - m_mousePos.y;

	m_mousePos.x = mousepos.x;
	m_mousePos.y = mousepos.y;

#endif

	DWORD dwResult;
	XINPUT_STATE xstate;
	for (i = 0; i < MAX_PADS; i++)
	{
		ZeroMemory(&xstate, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &xstate);

		if (dwResult == ERROR_SUCCESS)
		{
			m_xPadActive[i] = true;

			setXButton(i, (int)X_BUTTONS::DPAD_UP, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			setXButton(i, (int)X_BUTTONS::DPAD_DOWN, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			setXButton(i, (int)X_BUTTONS::DPAD_LEFT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			setXButton(i, (int)X_BUTTONS::DPAD_RIGHT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			setXButton(i, (int)X_BUTTONS::BTN_START, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			setXButton(i, (int)X_BUTTONS::BTN_BACK, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			setXButton(i, (int)X_BUTTONS::BTN_L3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			setXButton(i, (int)X_BUTTONS::BTN_R3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			setXButton(i, (int)X_BUTTONS::BTN_L, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			setXButton(i, (int)X_BUTTONS::BTN_R, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));

			setXButton(i, (int)X_BUTTONS::BTN_GUIDE, (xstate.Gamepad.wButtons & 0x0400));
			setXButton(i, (int)X_BUTTONS::BTN_UNKNOWN, (xstate.Gamepad.wButtons & 0x0800));

			setXButton(i, (int)X_BUTTONS::BTN_A, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			setXButton(i, (int)X_BUTTONS::BTN_B, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			setXButton(i, (int)X_BUTTONS::BTN_X, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			setXButton(i, (int)X_BUTTONS::BTN_Y, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y));

			if ((xstate.Gamepad.sThumbLX < INPUT_DEADZONE_L &&
				xstate.Gamepad.sThumbLX > -INPUT_DEADZONE_L) &&
				(xstate.Gamepad.sThumbLY < INPUT_DEADZONE_L &&
					xstate.Gamepad.sThumbLY > -INPUT_DEADZONE_L))
			{
				xstate.Gamepad.sThumbLX = 0;
				xstate.Gamepad.sThumbLY = 0;
			}

			if ((xstate.Gamepad.sThumbRX < INPUT_DEADZONE_R &&
				xstate.Gamepad.sThumbRX > -INPUT_DEADZONE_R) &&
				(xstate.Gamepad.sThumbRY < INPUT_DEADZONE_R &&
					xstate.Gamepad.sThumbRY > -INPUT_DEADZONE_R))
			{
				xstate.Gamepad.sThumbRX = 0;
				xstate.Gamepad.sThumbRY = 0;
			}

			m_analogState[i][0].x = xstate.Gamepad.sThumbLX;
			m_analogState[i][0].y = xstate.Gamepad.sThumbLY;
			m_analogState[i][1].x = xstate.Gamepad.sThumbRX;
			m_analogState[i][1].y = xstate.Gamepad.sThumbRY;
			m_analogState[i][2].x = xstate.Gamepad.bLeftTrigger;
			m_analogState[i][2].y = xstate.Gamepad.bRightTrigger;

		}
		else
		{
			if (m_xPadActive[i])
			{
				ZeroMemory(&m_buttonState[i], MAX_PAD_BUTTONS);
				m_xPadActive[i] = false;
			}
		}  
	} 
#if DIRECT_INPUT_ACTIVE
	
#if DI_KEY_MOUSE
	HRESULT hr;
	hr = g_pDIKeyboard->Poll();

	if (FAILED(hr))	
	{
		g_pDIKeyboard->Acquire();	
		g_pDIMouse->Acquire();		

		g_pDIKeyboard->Poll();		
	}

	BYTE currentKeys[MAX_KEYS];	
	hr = g_pDIKeyboard->GetDeviceState(MAX_KEYS, currentKeys);	

	if (SUCCEEDED(hr))
	{
		for (i = 0; i < MAX_KEYS; i++)
		{
			m_keyState[i] <<= 1;	

			if (currentKeys[i] & 0x80)
			{
				m_keyState[i] |= 0x01;
			}
		}
	}

	g_pDIMouse->Poll();
	DIMOUSESTATE2 currentMouse;
	hr = g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &currentMouse);

	if (SUCCEEDED(hr))
	{
		for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
		{
			m_mouseState[i] <<= 1;	
			
			if (currentMouse.rgbButtons[i] & 0x80)
			{
				m_mouseState[i] |= 0x01;
			}
		}

		m_mouseVect.x = currentMouse.lX;
		m_mouseVect.y = currentMouse.lY;

		POINT mousepos;
		GetCursorPos(&mousepos);
		ScreenToClient(g_hWnd, &mousepos);
		m_mousePos.x = mousepos.x;
		m_mousePos.y = mousepos.y;

		m_wheelPos.x = 0;	
		m_wheelPos.y = currentMouse.lZ;
	}
#endif

	DIJOYSTATE2 jstate;	
	int j;

	for (i = 0; i < MAX_DI_PADS; i++)
	{
		if (g_pDIGamePad[i])
		{
			g_pDIGamePad[i]->Poll();	

			if (FAILED(g_pDIGamePad[i]->GetDeviceState(sizeof(DIJOYSTATE2), &jstate)))
			{
				if (FAILED(g_pDIGamePad[i]->Acquire()))
				{
					releaseDIGameController(i);
				}
				else
				{
					g_pDIGamePad[i]->Poll();
				}

			}
			else
			{
				if ((jstate.lX < INPUT_DEADZONE_L &&
					jstate.lX > -INPUT_DEADZONE_L) &&
					(jstate.lY < INPUT_DEADZONE_L &&
						jstate.lY > -INPUT_DEADZONE_L) &&
					(jstate.lZ < INPUT_DEADZONE_L &&
						jstate.lZ > -INPUT_DEADZONE_L))
				{
					jstate.lX = 0;
					jstate.lY = 0;
					jstate.lZ = 0;
				}

				if ((jstate.lRx < INPUT_DEADZONE_R &&
					jstate.lRx > -INPUT_DEADZONE_R) &&
					(jstate.lRy < INPUT_DEADZONE_R &&
						jstate.lRy > -INPUT_DEADZONE_R) &&
					(jstate.lRz < INPUT_DEADZONE_R &&
						jstate.lRz > -INPUT_DEADZONE_R))
				{
					jstate.lRx = 0;
					jstate.lRy = 0;
					jstate.lRz = 0;
				}

				for (j = 0; j < MAX_DI_BUTTONS; j++)
				{
					setXButton(MAX_PADS + i, j, (jstate.rgbButtons[i] & 0x80));
				}

				for (j = 0; j < 4; j++)
				{
					m_diDirection[i][j] &= 0xf0;
					switch (jstate.rgdwPOV[j])
					{
					case -1:	
						break;

					case 0:		
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_UP;
						break;

					case 4500:	
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 9000:	
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_RIGHT;
						break;

					case 13500:	
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 18000:	
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_DOWN;
						break;

					case 22500:	
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_LEFT);
						break;

					case 27000:	
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_LEFT;
						break;

					case 31500:	
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_LEFT);
						break;
					}
				}

				m_diAnalogState[i][0].x = jstate.lX;
				m_diAnalogState[i][0].y = jstate.lY;
				m_diAnalogState[i][0].z = jstate.lZ;

				m_diAnalogState[i][1].x = jstate.lRx;
				m_diAnalogState[i][1].y = jstate.lRy;
				m_diAnalogState[i][1].z = jstate.lRz;

				m_diAnalogState[i][2].x = jstate.lAX;
				m_diAnalogState[i][2].y = jstate.lAY;
				m_diAnalogState[i][2].z = jstate.lAZ;
			}
		}
	}
#endif
}

void InputManager::refreshBuffer()
{
	int i, j;

	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] &= 0x01;	
	}

	m_wheelPos.x = 0;
	m_wheelPos.y = 0;
	m_wheelDelta = 0;
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] &= 0x01;	
	}
#endif

	for (i = 0; i < MAX_PADS; i++)
	{
		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j].x = 0;
			m_analogState[i][j].y = 0;
		}

		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] &= 0x01;
		}
	}
#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_diAnalogState[i][j].x = 0;
			m_diAnalogState[i][j].y = 0;
			m_diAnalogState[i][j].z = 0;
		}

		m_diDirection[i][0] <<= 4;
		m_diDirection[i][1] <<= 4;
		m_diDirection[i][2] <<= 4;
		m_diDirection[i][3] <<= 4;

		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] &= 0x01;
		}
	}
#endif
}

#if !DIRECT_INPUT_ACTIVE | !DI_KEY_MOUSE
void InputManager::mouseWheel(short delta, short x, short y)
{
	m_wheelPos.x = x;
	m_wheelPos.y = y;
	m_wheelDelta = delta;
}
#endif

bool InputManager::checkKeyboard(UINT keycode, BUTTON_STATE state)
{
	if (keycode >= MAX_KEYS)
		return false;

	return checkState(m_keyState[keycode], state);
}

bool InputManager::checkGamePad(UINT padId, UINT btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS)
	{
		if (m_xPadActive[padId] && btnId < MAX_PAD_BUTTONS)
			return checkState(m_buttonState[padId][btnId], state);

		return false;
	}
	else
	{
#if DIRECT_INPUT_ACTIVE
		padId -= MAX_PADS;

		if (padId < MAX_DI_PADS)
		{
			if (g_pDIGamePad[padId] && btnId < MAX_DI_BUTTONS)
				return checkState(m_diButtonState[padId][btnId], state);

			return false;

		}
#endif
		return false;
	}
}

#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
bool InputManager::checkMouseButton(UINT btnId, BUTTON_STATE state)
{
	if (btnId >= MAX_MOUSE_BUTTONS)
		return false;

	return checkState(m_mouseState[btnId], state);
}
#endif

XMINT2 InputManager::getMousePosition()
{
	return m_mousePos;
}

XMINT2 InputManager::getMouseVector()
{
	return m_mouseVect;
}

XMINT2 InputManager::getWheelPosition()
{
	return m_wheelPos;
}

int InputManager::getWheelDelta()
{
	return m_wheelDelta;
}

XMINT2 InputManager::getAnalogPosition(UINT padId, int analogId)
{
	XMINT2 res = {};

	if (padId < MAX_PADS && analogId < MAX_PAD_ANALOGIN && m_xPadActive[padId])
	{
		return m_analogState[padId][analogId];
	}

	return res;
}

void InputManager::setInputEnable(bool flg)
{
	if (flg)
	{
#if DIRECT_INPUT_ACTIVE
		
#endif
	}
	else
	{
#if DIRECT_INPUT_ACTIVE
#endif
		XINPUT_VIBRATION vib = {};

		for (int i = 0; i < MAX_PADS; i++)
			XInputSetState(i, &vib);
	}
}

#if DIRECT_INPUT_ACTIVE
HRESULT InputManager::initDirectInput(HINSTANCE hInst)
{

	HRESULT hr;

	if (!g_pDInput)
	{
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&g_pDInput, NULL);

		if (FAILED(hr))
		{
			return hr;
		}
	}

#if DI_KEY_MOUSE
	hr = g_pDInput->CreateDevice(GUID_SysKeyboard, &g_pDIKeyboard, NULL);
	if (FAILED(hr)) return hr;	

	hr = g_pDIKeyboard->SetDataFormat(&c_dfDIKeyboard);	
	if (FAILED(hr))	return hr;

	hr = g_pDIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))	return hr;


	hr = g_pDInput->CreateDevice(GUID_SysMouse, &g_pDIMouse, NULL);
	if (FAILED(hr)) return hr;	

	hr = g_pDIMouse->SetDataFormat(&c_dfDIMouse2);	 
	if (FAILED(hr))	return hr;

	hr = g_pDIMouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))	return hr;
#endif

	hr = g_pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACKW)EnumDIJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);
	
	if (FAILED(hr))	return hr;
	return hr;
}

void InputManager::releaseDirectInput()
{
	releaseAllDIControllers();
	if (g_pDInput)
	{
		g_pDInput->Release();
		g_pDInput = nullptr;
	}
}

bool InputManager::checkDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS && m_xPadActive[padId])
	{
		switch (btnId)
		{
		case X_BUTTONS::DPAD_UP:
		case X_BUTTONS::DPAD_RIGHT:
		case X_BUTTONS::DPAD_DOWN:
		case X_BUTTONS::DPAD_LEFT:
			return checkState(m_buttonState[padId][(UINT)btnId], state);
		}
	}
	else
	{
		if (povId < 4)
		{
			padId -= MAX_PADS;

			if (padId < MAX_DI_PADS && g_pDIGamePad[padId])
			{
				BYTE flg = m_diDirection[padId][povId];
				BYTE mask = 0;

				switch (btnId)
				{
				case X_BUTTONS::DPAD_UP:
					mask = (BYTE)DI_POV::DPOV_UP;
					break;
				case X_BUTTONS::DPAD_RIGHT:
					mask = (BYTE)DI_POV::DPOV_RIGHT;
					break;
				case X_BUTTONS::DPAD_DOWN:
					mask = (BYTE)DI_POV::DPOV_DOWN;
					break;
				case X_BUTTONS::DPAD_LEFT:
					mask = (BYTE)DI_POV::DPOV_LEFT;
					break;
				}

				switch (state)
				{
				case BUTTON_STATE::BUTTON_UP:
					mask = mask | (mask << 4);
					return (flg & mask) == (0xf0 & mask);	

				case BUTTON_STATE::BUTTON_DOWN:
					mask = mask | (mask << 4);
					return (flg & mask) == (0x0f & mask);	

				case BUTTON_STATE::BUTTON_PRESS:			
					return (flg & mask);
				}
			}
		}
	}

	return false;
}

XMINT3 InputManager::getDiAnalogPosition(UINT padId, int analogId)
{
	XMINT3 res = {};

	if (padId > MAX_PADS)
	{
		padId -= MAX_PADS;

		if (padId < MAX_DI_PADS)
		{
			if (analogId < MAX_PAD_ANALOGIN)
			{
				return m_diAnalogState[padId][analogId];
			}
		}
	}

	return res;
}

void InputManager::releaseDIGameController(int conId)
{
	if (conId < MAX_DI_PADS)
	{
		if (g_pDIGamePad[conId])
		{
			g_pDIGamePad[conId]->Unacquire();
			g_pDIGamePad[conId]->Release();
			g_pDIGamePad[conId] = nullptr;
		}
	}
}

void InputManager::releaseAllDIControllers()
{
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	if (g_pDIKeyboard != nullptr)
	{
		g_pDIKeyboard->Unacquire();
		g_pDIKeyboard->Release();
		g_pDIKeyboard = nullptr;
	}

	if (g_pDIMouse != nullptr)
	{
		g_pDIMouse->Unacquire();
		g_pDIMouse->Release();
		g_pDIMouse = nullptr;
	}
#endif

	for (int i = 0; i < MAX_DI_PADS; i++)
	{
		releaseDIGameController(i);
	}

	g_DICount = 0;
}

#endif

void InputManager::setXButton(UINT padId, UINT btnId, bool push)
{
	BYTE* targetState = nullptr;
	if (padId < MAX_PADS)
	{
		targetState = m_buttonState[padId];
	}
	else
	{
#if DIRECT_INPUT_ACTIVE
		padId -= MAX_PADS;
		if (padId < MAX_DI_PADS)
		{
			targetState = m_diButtonState[padId];
		}
		else
		{
			return;
		}
#else
		return;
#endif
	}

	targetState[btnId] <<= 1;	

	if (push)
	{
		targetState[btnId] |= 0x01;		
	}
}

bool InputManager::checkState(UINT flg, BUTTON_STATE state)
{
	switch (state)
	{
	case BUTTON_STATE::BUTTON_PRESS:
		return (flg == 0x01);
	case BUTTON_STATE::BUTTON_DOWN:
		return ((flg & 0x01) == 0x01);
	case BUTTON_STATE::BUTTON_UP:
		return (flg == 0x02);
	}
	return false;
}
