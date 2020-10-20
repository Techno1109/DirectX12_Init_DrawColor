#include "Dx12Wrapper.h"
#include "Application.h"

#include <assert.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

//using namespace DirectX;


Dx12Wrapper::Dx12Wrapper(HWND windowH) :_WindowH(windowH)
{
	//DXGI
	_DxgiFactory = nullptr;
	_DxgiSwapChain = nullptr;

	//デバイス
	_Dev = nullptr;

	//コマンド系
	//コマンドリスト確保用オブジェクト
	_CmdAlloc = nullptr;
	//コマンドリスト本体
	_CmdList = nullptr;
	//コマンドリストのキュー
	_CmdQueue = nullptr;

	//フェンスオブジェクト
	_Fence = nullptr;
}

Dx12Wrapper::~Dx12Wrapper()
{
}

bool Dx12Wrapper::Init()
{
#if _DEBUG
	ID3D12Debug* Debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
	Debug->EnableDebugLayer();
	Debug->Release();
#endif

	HRESULT Result = S_OK;

	D3D_FEATURE_LEVEL Levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	for (auto level : Levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(_Dev.ReleaseAndGetAddressOf()))))
		{
			break;
		}
	}


	if (_Dev == nullptr)
	{
		return false;
	}

#if _DEBUG

	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_DxgiFactory.ReleaseAndGetAddressOf()))))
	{
		return false;
	}
#else

	if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(_DxgiFactory.ReleaseAndGetAddressOf()))))
	{
		return false;
	}
#endif

	//コマンドキューを作成

	D3D12_COMMAND_QUEUE_DESC CmdQueueDesc{};
	CmdQueueDesc.NodeMask = 0;
	CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;


	if (FAILED(_Dev->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(_CmdQueue.ReleaseAndGetAddressOf()))))
	{
		return false;
	}


	//スワップチェインを作成
	Size WindowSize = Application::Instance().GetWindowSize();

	DXGI_SWAP_CHAIN_DESC1 SwDesc = {};
	SwDesc.BufferCount = 2;
	SwDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwDesc.Flags = 0;
	SwDesc.Width = WindowSize.Width;
	SwDesc.Height = WindowSize.Height;
	SwDesc.SampleDesc.Count = 1;
	SwDesc.SampleDesc.Quality = 0;
	SwDesc.Stereo = false;
	SwDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwDesc.Scaling = DXGI_SCALING_STRETCH;

	Result = _DxgiFactory->CreateSwapChainForHwnd(_CmdQueue.Get(), _WindowH, &SwDesc, nullptr, nullptr, (IDXGISwapChain1**)(_DxgiSwapChain.ReleaseAndGetAddressOf()));

	if (FAILED(Result))
	{
		return false;
	}


	//フェンスを作成
	Result = _Dev->CreateFence(_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_Fence.ReleaseAndGetAddressOf()));

	if (FAILED(Result))
	{
		return false;
	}


	Result = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(Result))
	{
		return false;
	}

	if (!CreateCommandList())
	{
		return false;
	}

	if (!CreateRenderTargetView())
	{
		return false;
	}



	return true;
}

bool Dx12Wrapper::ScreenCrear()
{
	auto BackBufferIdx = _DxgiSwapChain->GetCurrentBackBufferIndex();
	auto HeapPointer = _Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	HeapPointer.ptr += BackBufferIdx * _Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//バリア追加
	AddBarrier(_BackBuffers[_DxgiSwapChain->GetCurrentBackBufferIndex()], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//レンダリング
	_CmdList->OMSetRenderTargets(1, &HeapPointer, false, nullptr);

	//RGBA指定　最大1
	float ClsColor[4] = { 1,0, 0,0.0 };
	_CmdList->ClearRenderTargetView(HeapPointer, ClsColor, 0, nullptr);

	return true;
}

void Dx12Wrapper::DrawSetting()
{
}

void Dx12Wrapper::EndDraw()
{
}

void Dx12Wrapper::Draw()
{
	AddBarrier(_BackBuffers[_DxgiSwapChain->GetCurrentBackBufferIndex()], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_CmdList->Close();
	ID3D12CommandList* Cmds[] = { _CmdList.Get() };
	_CmdQueue->ExecuteCommandLists(1, Cmds);
	++_FenceValue;

	_CmdQueue->Signal(_Fence.Get(), _FenceValue);
	WaitWithFence();

	_CmdAlloc->Reset();
	//ここで全部のPipeLineをリセットできたらいいな
	_CmdList->Reset(_CmdAlloc.Get(), nullptr);
}

void Dx12Wrapper::ScreenFlip()
{
	HRESULT Result = S_OK;
	Result = _DxgiSwapChain->Present(1, 0);
	assert(SUCCEEDED(Result));
}

void Dx12Wrapper::WaitWithFence()
{
	//ポーリング待機
	while (_Fence->GetCompletedValue() != _FenceValue)
	{
		auto Event = CreateEvent(nullptr, false, false, nullptr);
		_Fence->SetEventOnCompletion(_FenceValue, Event);
		WaitForSingleObject(Event, INFINITE);	//ここで待機。
		CloseHandle(Event);
	}
}

void Dx12Wrapper::AddBarrier(Microsoft::WRL::ComPtr<ID3D12Resource>& Buffer, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	//リソースバリアを設定
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.StateBefore = StateBefore;
	BarrierDesc.Transition.StateAfter = StateAfter;
	BarrierDesc.Transition.pResource = Buffer.Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	//レンダリング前にバリアを追加
	_CmdList->ResourceBarrier(1, &BarrierDesc);
}

bool Dx12Wrapper::CreateCommandList()
{
	HRESULT Result = S_OK;

	//コマンドアロケータ作成
	Result = _Dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_CmdAlloc.ReleaseAndGetAddressOf()));

	if (FAILED(Result))
	{
		return false;
	}

	//コマンドリスト作成
	Result = _Dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _CmdAlloc.Get(), nullptr, IID_PPV_ARGS(_CmdList.ReleaseAndGetAddressOf()));

	if (FAILED(Result))
	{
		return false;
	}

	return true;
}

bool Dx12Wrapper::CreateRenderTargetView()
{
	HRESULT Result = S_OK;

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;

	Result = _DxgiSwapChain->GetDesc1(&SwapChainDesc);

	if (FAILED(Result))
	{
		return false;
	}

	//表示画面メモリ確保(デスクリプタヒープ作成)
	D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc{};
	DescripterHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescripterHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DescripterHeapDesc.NodeMask = 0;
	DescripterHeapDesc.NumDescriptors = SwapChainDesc.BufferCount;

	Result = _Dev->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(_Rtv_DescriptorHeap.ReleaseAndGetAddressOf()));

	if (FAILED(Result))
	{
		return false;
	}

	_BackBuffers.resize(SwapChainDesc.BufferCount);

	int DescripterSize = _Dev->GetDescriptorHandleIncrementSize(DescripterHeapDesc.Type);

	D3D12_CPU_DESCRIPTOR_HANDLE DescripterHandle = _Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
	RtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//レンダーターゲットビューをヒープに紐づけ
	for (int i = 0; i < _BackBuffers.size(); i++)
	{
		Result = _DxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&_BackBuffers[i]));

		if (FAILED(Result))
		{
			break;
		}

		_Dev->CreateRenderTargetView(_BackBuffers[i].Get(), nullptr, DescripterHandle);
		DescripterHandle.ptr += DescripterSize;
	}

	if (FAILED(Result))
	{
		return false;
	}

	return true;
}
