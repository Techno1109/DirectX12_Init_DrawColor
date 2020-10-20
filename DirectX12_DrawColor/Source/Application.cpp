#include "Application.h"
#include "Dx12Wrapper.h"
#include <iostream>

constexpr int Window_W = 1280;
constexpr int Window_H = 720;


//コールバック関数＿OSから呼び出されるので定義が必要。
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);

		return 0;
	}
	return DefWindowProc(hwnd,msg,wparam,lparam);
}


Application::Application()
{
}


Application & Application::Instance()
{
	static Application instance;
	return instance;
}

bool Application::Init()
{
	_WndClass.hInstance = GetModuleHandle(nullptr);
	_WndClass.cbSize=sizeof(WNDCLASSEX);
	_WndClass.lpfnWndProc=(WNDPROC)WindowProcedure;
	_WndClass.lpszClassName = "DirectX12_DrawColor";
	RegisterClassEx(&_WndClass);

	RECT Wrect = {0,0,Window_W,Window_H};
	AdjustWindowRect(&Wrect, WS_OVERLAPPEDWINDOW, false);
	_Hwnd = CreateWindow
		(
		_WndClass.lpszClassName,
		"DirectX12_DrawColor_TechNO",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Wrect.right-Wrect.left,
		Wrect.bottom - Wrect.top,
		nullptr,
		nullptr,
		_WndClass.hInstance,
		nullptr
		);


	if (_Hwnd == 0)
	{
		return false;
	}

	_Dx12 = std::make_shared<Dx12Wrapper>(_Hwnd);

	if (!_Dx12->Init())
	{
		return false;
	}

	return true;
}

void Application::Run()
{
	ShowWindow(_Hwnd,SW_SHOW);
	MSG msg = {};

	while (true)
	{
		//OSからメッセージを受け取る
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//受け取ったメッセージそのままでは情報不足、Translateする
			TranslateMessage(&msg);
			//変換されなかったメッセージはOSへそのまま返す。
			DispatchMessage(&msg);
		}


		//QUITメッセージが来ていたらブレイク。
		if (msg.message==WM_QUIT)
		{
			break;
		}
	
		_Dx12->DrawSetting();

		//描画終了(コマンド実行)
		_Dx12->EndDraw();
		//描画前設定
		_Dx12->ScreenCrear();
		_Dx12->Draw();
		//画面の更新
		_Dx12->ScreenFlip();
	}
}

void Application::Terminate()
{
	CoUninitialize();
	//Windowの使用権を返却
	UnregisterClass(_WndClass.lpszClassName,_WndClass.hInstance);
}

Size Application::GetWindowSize()
{
	return Size(Window_W,Window_H);
}

Application::~Application()
{
}
