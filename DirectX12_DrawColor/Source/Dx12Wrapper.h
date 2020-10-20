#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <map>
#include <memory>
#include <wrl/client.h>

//DirectX12�̏��������̊e�v�f�����܂Ƃ߂邽�߂̃N���X
class Dx12Wrapper
{
public:
	Dx12Wrapper(HWND windowH);
	~Dx12Wrapper();

	bool Init();

	//��ʂ̃��Z�b�g
	bool ScreenCrear();
	//�`��O�ݒ�
	void DrawSetting();
	//�`��I��(�R�}���h���s)
	void EndDraw();
	//1stPath�`��
	void Draw();
	//��ʂ̍X�V
	void ScreenFlip();

private:

	//�E�B���h�E�n���h��
	HWND _WindowH;

	//�f�o�C�X-----------
	Microsoft::WRL::ComPtr<ID3D12Device> _Dev;


	//�R�}���h���X�g�쐬
	bool CreateCommandList();
	//�R�}���h���X�g�m�ۗp�I�u�W�F�N�g
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> _CmdAlloc;
	//�R�}���h���X�g�{��
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> _CmdList;
	//�R�}���h���X�g�̃L���[
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> _CmdQueue;

	//DXGI-----------
	Microsoft::WRL::ComPtr<IDXGIFactory6> _DxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _DxgiSwapChain;

	//�t�F���X�I�u�W�F�N�g-----------
	Microsoft::WRL::ComPtr <ID3D12Fence> _Fence;
	//�t�F���X�l
	UINT64 _FenceValue;
	//�|�[�����O�ҋ@
	void WaitWithFence();
	//�o���A�ǉ�
	void AddBarrier(Microsoft::WRL::ComPtr<ID3D12Resource>& Buffer, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	//�����_�[�^�[�Q�b�gView�쐬
	bool CreateRenderTargetView();
	//�����_�[�^�[�Q�b�g�p�f�X�N���v�^�q�[�v
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> _Rtv_DescriptorHeap;
	//�X���b�v�`�F�C���̃��\�[�X�|�C���^�Q
	std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> _BackBuffers;

};
