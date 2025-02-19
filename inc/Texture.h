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
 * Texture クラス
 ****************************************************************/
class Texture {

private:

	ComPtr<ID3D12Resource>      m_pResource;  /* リソース */
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCPU;  /* CPUのディスクリプタに対するハンドル */
	D3D12_GPU_DESCRIPTOR_HANDLE m_HandleGPU;  /* GPUのディスクリプタに対するハンドル */

public:

	Texture();

	~Texture();

	/****************************************************************
	 * 初期化処理
	 ****************************************************************/
	bool Init(
		ID3D12Device*                 pDevice, 
		ID3D12DescriptorHeap*         pHeap, 
		const wchar_t*                filename, 
		DirectX::ResourceUploadBatch& batch,
		size_t                        HeapCount);


	/****************************************************************
	 * 終了処理
	 ****************************************************************/
	void Term();


	/****************************************************************
	 * テクスチャのGPUハンドルを取得
	 ****************************************************************/
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const;

};





