#pragma once
#include<memory>
#include<vector>
#include <Windows.h>

struct Size
{
	int Height;
	int Width;
	Size() {};
	Size(int w, int h) :Width(w), Height(h) {};
};

class Dx12Wrapper;
class Application
{

public:
	//�V���O���g���p�^�[��
	static Application& Instance();
	//������
	bool Init();
	//���s
	void Run();
	//�I������
	void Terminate();

	Size GetWindowSize();

	~Application();

private:
	Application();
	Application(const Application&);
	void operator=(const Application&);

	std::shared_ptr<Dx12Wrapper> _Dx12;

	HWND _Hwnd;
	WNDCLASSEX _WndClass;
};

