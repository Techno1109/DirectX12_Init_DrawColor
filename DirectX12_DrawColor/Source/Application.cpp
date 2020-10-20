#include "Application.h"
#include "Dx12Wrapper.h"
#include <iostream>

constexpr int Window_W = 1280;
constexpr int Window_H = 720;


//�R�[���o�b�N�֐��QOS����Ăяo�����̂Œ�`���K�v�B
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
		//OS���烁�b�Z�[�W���󂯎��
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//�󂯎�������b�Z�[�W���̂܂܂ł͏��s���ATranslate����
			TranslateMessage(&msg);
			//�ϊ�����Ȃ��������b�Z�[�W��OS�ւ��̂܂ܕԂ��B
			DispatchMessage(&msg);
		}


		//QUIT���b�Z�[�W�����Ă�����u���C�N�B
		if (msg.message==WM_QUIT)
		{
			break;
		}
	
		_Dx12->DrawSetting();

		//�`��I��(�R�}���h���s)
		_Dx12->EndDraw();
		//�`��O�ݒ�
		_Dx12->ScreenCrear();
		_Dx12->Draw();
		//��ʂ̍X�V
		_Dx12->ScreenFlip();
	}
}

void Application::Terminate()
{
	CoUninitialize();
	//Window�̎g�p����ԋp
	UnregisterClass(_WndClass.lpszClassName,_WndClass.hInstance);
}

Size Application::GetWindowSize()
{
	return Size(Window_W,Window_H);
}

Application::~Application()
{
}
