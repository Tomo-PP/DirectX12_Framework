#pragma once
/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <string>
#include "ComPtr.h"
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
#include "VertexTypes.h"


/****************************************************************
 * Texture �N���X
 ****************************************************************/
class Texture {

private:

	ComPtr<ID3D12Resource>      m_pResource;  /* ���\�[�X */
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCPU;  /* CPU�̃f�B�X�N���v�^�ɑ΂���n���h�� */
	D3D12_GPU_DESCRIPTOR_HANDLE m_HandleGPU;  /* GPU�̃f�B�X�N���v�^�ɑ΂���n���h�� */

public:

	Texture();

	~Texture();

	/****************************************************************
	 * ����������
	 ****************************************************************/
	bool Init(
		ID3D12Device*                 pDevice, 
		ID3D12DescriptorHeap*         pHeap, 
		const wchar_t*                filename, 
		DirectX::ResourceUploadBatch& batch,
		size_t                        HeapCount);


	/****************************************************************
	 * �I������
	 ****************************************************************/
	void Term();


	/****************************************************************
	 * �e�N�X�`����GPU�n���h�����擾
	 ****************************************************************/
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const;

};





