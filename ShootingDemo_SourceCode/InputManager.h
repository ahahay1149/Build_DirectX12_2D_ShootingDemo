#pragma once

#include <Windows.h>		
#include <directxmath.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

#define DIRECT_INPUT_ACTIVE 1	
#define DI_KEY_MOUSE 0			

#if DIRECT_INPUT_ACTIVE
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

#define MAX_DI_PADS 4			
#define MAX_DI_BUTTONS 32		
#endif


#define MAX_KEYS 256			 
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
#define MAX_MOUSE_BUTTONS 5		
#endif
#define MAX_PAD_BUTTONS 16		
#define MAX_PAD_ANALOGIN 3		
#define MAX_PADS 4				

#define INPUT_DEADZONE_L  ( 0.24f * FLOAT(0x7FFF) )                        
#define INPUT_DEADZONE_R  ( 0.24f * FLOAT(0x7FFF) )                        

using namespace DirectX;

class InputManager
{
public:

	enum class BUTTON_STATE
	{
		BUTTON_DOWN,
		BUTTON_PRESS,
		BUTTON_UP
	};

	enum class X_BUTTONS
	{
		DPAD_UP = 0,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		BTN_START,
		BTN_BACK,
		BTN_L3,
		BTN_R3,
		BTN_L,
		BTN_R,

		BTN_GUIDE,			 	
		BTN_UNKNOWN,		

		BTN_A,
		BTN_B,
		BTN_X,
		BTN_Y,
	};

	static InputManager* getInstance();	
	~InputManager();					

	HRESULT initInputManager(HINSTANCE hInst, HWND hwnd);	
	void update();						
	void refreshBuffer();				

#if !DIRECT_INPUT_ACTIVE | !DI_KEY_MOUSE
	void mouseWheel(short delta, short x, short y);
#endif

	bool checkKeyboard(UINT keycode, BUTTON_STATE state);
	bool checkGamePad(UINT padId, UINT btnId, BUTTON_STATE state);
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	bool checkMouseButton(UINT btnId, BUTTON_STATE state);
#endif

	XMINT2	getMousePosition();
	XMINT2	getMouseVector();
	XMINT2	getWheelPosition();
	int		getWheelDelta();
	XMINT2	getAnalogPosition(UINT padId, int analogId);

	void setInputEnable(bool flg);
#if DIRECT_INPUT_ACTIVE
	void releaseDirectInput();

	bool checkDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state);
	XMINT3 getDiAnalogPosition(UINT padId, int analogId);
#endif

protected:

#if DIRECT_INPUT_ACTIVE
	HRESULT initDirectInput(HINSTANCE hInst);

	enum class DI_POV
	{
		DPOV_UP = 0x01,
		DPOV_RIGHT = 0x02,
		DPOV_DOWN = 0x04,
		DPOV_LEFT = 0x08,
	};
	BYTE m_diDirection[MAX_DI_PADS][4];	
	BYTE m_diButtonState[MAX_DI_PADS][MAX_DI_BUTTONS];
	XMINT3 m_diAnalogState[MAX_DI_PADS][MAX_PAD_ANALOGIN];
#endif

	BYTE m_keyState[MAX_KEYS];				
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	BYTE m_mouseState[MAX_MOUSE_BUTTONS];	Å@
#endif
	BYTE m_buttonState[MAX_PADS][MAX_PAD_BUTTONS];		
	XMINT2 m_analogState[MAX_PADS][MAX_PAD_ANALOGIN];	

	bool m_xPadActive[MAX_PADS];				

	XMINT2	m_mousePos = { 0, 0 };			
	XMINT2	m_mouseVect = { 0, 0 };			
	XMINT2	m_wheelPos = { 0, 0 };			
	int		m_wheelDelta;					

	void setXButton(UINT padId, UINT btnId, bool push);
	bool checkState(UINT flg, BUTTON_STATE state);
#if DIRECT_INPUT_ACTIVE
	void releaseDIGameController(int conId);
	void releaseAllDIControllers();
#endif

	InputManager();

};