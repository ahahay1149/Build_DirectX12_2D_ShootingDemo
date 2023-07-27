#include <windowsx.h>
#include "MyAppRunner.h"
#include "SceneController.h"
#include "InputManager.h"

HWND MyAppRunner::m_hWnd = nullptr; 
bool MyAppRunner::m_fullscreenMode = false;
RECT MyAppRunner::m_windowRect;

int MyAppRunner::Run(MyGameEngine* pMyEngine, HINSTANCE hInstance, int nCmdShow)
{
    try
    {
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"MyGameEngineSample";
        RegisterClassEx(&windowClass);

        m_windowRect = { 0, 0, static_cast<LONG>(pMyEngine->getWidth()), static_cast<LONG>(pMyEngine->getHeight()) };
        AdjustWindowRect(&m_windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hWnd = CreateWindow(
            windowClass.lpszClassName,
            pMyEngine->getTitle(),
            m_windowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            m_windowRect.right - m_windowRect.left,
            m_windowRect.bottom - m_windowRect.top,
            nullptr,             
            nullptr,            
            hInstance,
            pMyEngine);

        if (FAILED(pMyEngine->initMyGameEngine(hInstance, m_hWnd))) return 0;

        ShowWindow(m_hWnd, nCmdShow);

        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        pMyEngine->CleanupDevice();

        return static_cast<char>(msg.wParam);
    }
    catch (std::exception& e)
    {
        OutputDebugString(L"Application hit a problem: ");
        OutputDebugStringA(e.what());
        OutputDebugString(L"\nTerminating.\n");

        pMyEngine->CleanupDevice();
        return EXIT_FAILURE;
    }
}

LRESULT MyAppRunner::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MyGameEngine* pEngine = reinterpret_cast<MyGameEngine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_COMMAND:
    {
    }
    break;

    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    break;
    case WM_PAINT:
        pEngine->FrameUpdate();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_MOUSEWHEEL:
    {
        InputManager* inputMng = InputManager::getInstance();

        inputMng->mouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}
