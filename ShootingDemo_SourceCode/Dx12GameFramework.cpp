#include "stdafx.h"
#include "DXSampleHelper.h"
#include "MyAppRunner.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    MyGameEngine* engine = MyGameEngine::getInstance();         
    engine->setEngineWH(960, 540, L"DX12GameFrameworkBase");    

    return MyAppRunner::Run(engine, hInstance, nCmdShow);       
}