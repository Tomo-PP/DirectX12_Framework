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
 * クラス
 ****************************************************************/
class ConstantBuffer;


/****************************************************************
 * DescriptorManager クラス
 ****************************************************************/
class DescriptorManager {

private:

	static const uint32_t _FrameCount = 2;
	
	size_t m_HeapCount;   /* ヒープ生成を行ったサイズ */

	
	/****************************************************************
	 * リソース（バッファ・ディスクリプタ）
	 ****************************************************************/
	ComPtr<ID3D12Resource>  m_pRTB[_FrameCount];  /* レンダーターゲットのバッファ（カラーターゲットバッファ）*/
	ComPtr<ID3D12Resource>  m_pDSB;               /* 深度ステンシルバッファ */
	
	
	/****************************************************************
	 * ディスクリプタヒープ
	 ****************************************************************/
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;          /* レンダーターゲットビュー用 */
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;          /* 深度ステンシルビュー用 */
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV_SRV_UAV;  /* CBV・SRV・UAV用 */
	ComPtr<ID3D12DescriptorHeap> m_pHeapGlobal;       /* シーンで共通して使うグローバルヒープ */


	/****************************************************************
	 * ビュー
	 ****************************************************************/
	D3D12_CPU_DESCRIPTOR_HANDLE  m_HandleRTV[_FrameCount];  /* RTV（CPUハンドルのみ）*/
	D3D12_CPU_DESCRIPTOR_HANDLE  m_HandleDSV;               /* DSV（CPUハンドルのみ）*/


	/****************************************************************
	 * 生成カウントのインクリメント（CBV・SRV・UAV作成時に処理）
	 ****************************************************************/
	void AddCount()
	{ m_HeapCount++; };


public:

	DescriptorManager();

	~DescriptorManager();


	/****************************************************************
	 * RTVの作成（ディスクリプターヒープ）
	 ****************************************************************/
	bool CreateRTV(ID3D12Device* pDevice, IDXGISwapChain3* pSwapChain);


	/****************************************************************
	 * DSVの作成（ディスクリプターヒープ）
	 ****************************************************************/
	bool CreateDSV(ID3D12Device* pDevice, uint32_t Width, uint32_t Height);


	/****************************************************************
	 * CBV_SRV_UAVのディスクリプタヒープの生成
	 ****************************************************************/
	bool Init_CBV_SRV_UAV(ID3D12Device* pDevice, size_t HeapSize);


	/****************************************************************
	 * グローバルディスクリプタヒープの生成
	 ****************************************************************/
	bool Init_GlobalHeap(ID3D12Device* pDevice, size_t HeapSize);


	/****************************************************************
	 * CBVの作成
	 ****************************************************************/
	bool CreateCBV(
		ID3D12Device*         pDevice,
		ID3D12DescriptorHeap* pHeap,
		ConstantBuffer*       pCBV,
		size_t                size);


	/****************************************************************
	 * SRVの作成
	 ****************************************************************/
	bool CreateSRV(
		ID3D12Device*                 pDevice, 
		ID3D12DescriptorHeap*         pHeap, 
		Texture*                      pSRV,
		const wchar_t*                filename,
		DirectX::ResourceUploadBatch& batch);


	/****************************************************************
	 * ディスクリプタヒープのコピー
	 ****************************************************************/
	bool CopyDescriptorHeap(
		ID3D12Device*               pDevice, 
		ID3D12DescriptorHeap*       localHeap,
		D3D12_CPU_DESCRIPTOR_HANDLE srcHeapHandle);


	/****************************************************************
	 * CPUハンドルの取得
	 ****************************************************************/
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle_RTV(const uint32_t FrameIndex) const;    /* RTV */
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle_DSV() const;                             /* DSV */


	/****************************************************************
	 * リソースの取得
	 ****************************************************************/
	ID3D12Resource* GetResource_RTB(const uint32_t FrameIndex) const
	{ return m_pRTB[FrameIndex].Get(); };


	/****************************************************************
	 * ディスクリプタヒープの取得
	 ****************************************************************/
	ID3D12DescriptorHeap* GetHeapCBV_SRV_UAV()
	{ return m_pHeapCBV_SRV_UAV.Get(); }

	ID3D12DescriptorHeap* GetGlobalHeap()
	{ return m_pHeapGlobal.Get(); }


	/****************************************************************
	 * 終了処理
	 ****************************************************************/
	void Term();


private:

	/****************************************************************
	 * RTV ディスクリプターヒープの破棄
	 ****************************************************************/
	void TermRTV();



	/****************************************************************
	 * DSV ディスクリプターヒープの破棄
	 ****************************************************************/
	void TermDSV();


};