#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <map>
#include <memory>
#include <wrl/client.h>

//DirectX12の初期化等の各要素等をまとめるためのクラス
class Dx12Wrapper
{
public:
	Dx12Wrapper(HWND windowH);
	~Dx12Wrapper();

	bool Init();

	//画面のリセット
	bool ScreenCrear();
	//描画前設定
	void DrawSetting();
	//描画終了(コマンド実行)
	void EndDraw();
	//1stPath描画
	void Draw();
	//画面の更新
	void ScreenFlip();

private:

	//ウィンドウハンドル
	HWND _WindowH;

	//デバイス-----------
	Microsoft::WRL::ComPtr<ID3D12Device> _Dev;


	//コマンドリスト作成
	bool CreateCommandList();
	//コマンドリスト確保用オブジェクト
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> _CmdAlloc;
	//コマンドリスト本体
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> _CmdList;
	//コマンドリストのキュー
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> _CmdQueue;

	//DXGI-----------
	Microsoft::WRL::ComPtr<IDXGIFactory6> _DxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _DxgiSwapChain;

	//フェンスオブジェクト-----------
	Microsoft::WRL::ComPtr <ID3D12Fence> _Fence;
	//フェンス値
	UINT64 _FenceValue;
	//ポーリング待機
	void WaitWithFence();
	//バリア追加
	void AddBarrier(Microsoft::WRL::ComPtr<ID3D12Resource>& Buffer, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	//レンダーターゲットView作成
	bool CreateRenderTargetView();
	//レンダーターゲット用デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> _Rtv_DescriptorHeap;
	//スワップチェインのリソースポインタ群
	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> _BackBuffers;

};
