#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <dxgi1_4.h>
#include <cstdint>
#include <iostream>
#include "ComPtr.h"
#include "ConstantBuffer.h"
#include "Texture.h"



/****************************************************************
 * �N���X
 ****************************************************************/
class ConstantBuffer;


/****************************************************************
 * DescriptorManager �N���X
 ****************************************************************/
class DescriptorManager {

private:

	static const uint32_t _FrameCount = 2;
	
	size_t m_HeapCount;   /* �q�[�v�������s�����T�C�Y */

	
	/****************************************************************
	 * ���\�[�X�i�o�b�t�@�E�f�B�X�N���v�^�j
	 ****************************************************************/
	ComPtr<ID3D12Resource>  m_pRTB[_FrameCount];  /* �����_�[�^�[�Q�b�g�̃o�b�t�@�i�J���[�^�[�Q�b�g�o�b�t�@�j*/
	ComPtr<ID3D12Resource>  m_pDSB;               /* �[�x�X�e���V���o�b�t�@ */
	
	
	/****************************************************************
	 * �f�B�X�N���v�^�q�[�v
	 ****************************************************************/
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;          /* �����_�[�^�[�Q�b�g�r���[�p */
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;          /* �[�x�X�e���V���r���[�p */
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV_SRV_UAV;  /* CBV�ESRV�EUAV�p */
	ComPtr<ID3D12DescriptorHeap> m_pHeapGlobal;       /* �V�[���ŋ��ʂ��Ďg���O���[�o���q�[�v */


	/****************************************************************
	 * �r���[
	 ****************************************************************/
	D3D12_CPU_DESCRIPTOR_HANDLE  m_HandleRTV[_FrameCount];  /* RTV�iCPU�n���h���̂݁j*/
	D3D12_CPU_DESCRIPTOR_HANDLE  m_HandleDSV;               /* DSV�iCPU�n���h���̂݁j*/


	/****************************************************************
	 * �����J�E���g�̃C���N�������g�iCBV�ESRV�EUAV�쐬���ɏ����j
	 ****************************************************************/
	void AddCount()
	{ m_HeapCount++; };


public:

	DescriptorManager();

	~DescriptorManager();


	/****************************************************************
	 * RTV�̍쐬�i�f�B�X�N���v�^�[�q�[�v�j
	 ****************************************************************/
	bool CreateRTV(ID3D12Device* pDevice, IDXGISwapChain3* pSwapChain);


	/****************************************************************
	 * DSV�̍쐬�i�f�B�X�N���v�^�[�q�[�v�j
	 ****************************************************************/
	bool CreateDSV(ID3D12Device* pDevice, uint32_t Width, uint32_t Height);


	/****************************************************************
	 * CBV_SRV_UAV�̃f�B�X�N���v�^�q�[�v�̐���
	 ****************************************************************/
	bool Init_CBV_SRV_UAV(ID3D12Device* pDevice, size_t HeapSize);


	/****************************************************************
	 * �O���[�o���f�B�X�N���v�^�q�[�v�̐���
	 ****************************************************************/
	bool Init_GlobalHeap(ID3D12Device* pDevice, size_t HeapSize);


	/****************************************************************
	 * CBV�̍쐬
	 ****************************************************************/
	bool CreateCBV(
		ID3D12Device*         pDevice,
		ID3D12DescriptorHeap* pHeap,
		ConstantBuffer*       pCBV,
		size_t                size);


	/****************************************************************
	 * SRV�̍쐬
	 ****************************************************************/
	bool CreateSRV(
		ID3D12Device*                 pDevice, 
		ID3D12DescriptorHeap*         pHeap, 
		Texture*                      pSRV,
		const wchar_t*                filename,
		DirectX::ResourceUploadBatch& batch);


	/****************************************************************
	 * �f�B�X�N���v�^�q�[�v�̃R�s�[
	 ****************************************************************/
	bool CopyDescriptorHeap(
		ID3D12Device*               pDevice, 
		ID3D12DescriptorHeap*       localHeap,
		D3D12_CPU_DESCRIPTOR_HANDLE srcHeapHandle);


	/****************************************************************
	 * CPU�n���h���̎擾
	 ****************************************************************/
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle_RTV(const uint32_t FrameIndex) const;    /* RTV */
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle_DSV() const;                             /* DSV */


	/****************************************************************
	 * ���\�[�X�̎擾
	 ****************************************************************/
	ID3D12Resource* GetResource_RTB(const uint32_t FrameIndex) const
	{ return m_pRTB[FrameIndex].Get(); };


	/****************************************************************
	 * �f�B�X�N���v�^�q�[�v�̎擾
	 ****************************************************************/
	ID3D12DescriptorHeap* GetHeapCBV_SRV_UAV()
	{ return m_pHeapCBV_SRV_UAV.Get(); }

	ID3D12DescriptorHeap* GetGlobalHeap()
	{ return m_pHeapGlobal.Get(); }


	/****************************************************************
	 * �I������
	 ****************************************************************/
	void Term();


private:

	/****************************************************************
	 * RTV �f�B�X�N���v�^�[�q�[�v�̔j��
	 ****************************************************************/
	void TermRTV();



	/****************************************************************
	 * DSV �f�B�X�N���v�^�[�q�[�v�̔j��
	 ****************************************************************/
	void TermDSV();


};