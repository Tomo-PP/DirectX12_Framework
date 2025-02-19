
/****************************************************************
 * Include
 ****************************************************************/
#include "ConstantBuffer.h"
#include <iostream>

ConstantBuffer::ConstantBuffer() :
	m_pCB       (nullptr),
	m_HandleCPU (),
	m_HandleGPU (),
	m_Desc      (),
	m_pMapBuf   (nullptr)
{ /* DO_NOTHING */ }


ConstantBuffer::~ConstantBuffer()
{ /* DO_NOTHING */ }


bool ConstantBuffer::Init(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap, size_t size, size_t HeapCount) {

	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;    // アライメントサイズ（256 Byte）
	uint64_t alignmentSize = (size + (align - 1)) & ~(align - 1);   // アライメントサイズの計算

	// ヒーププロパティ（データをどう送るのかを記述）
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（CPUの書き込み方法について・今回は指定なし）*/
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */

	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定） */
	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	resourceDesc.Width              = alignmentSize;                    /* データサイズ：256 Byte（256 Byteを超える場合 512 Byte）*/
	resourceDesc.Height             = 1;                                /* データの縦のサイズ（バッファなので１）*/
	resourceDesc.DepthOrArraySize   = 1;                                /* データの奥行（バッファなので１）*/
	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定（今回は使わないので０） */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* 始まりから終わりまで連続したバッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する） */


	// ディスクリプタヒープ内のデータについて、次のデータに移動するためのインクリメントサイズを取得
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,                               /* ヒープの設定 */
		D3D12_HEAP_FLAG_NONE,                    /* ヒープのオプション */
		&resourceDesc,                           /* リソースの設定 */
		D3D12_RESOURCE_STATE_GENERIC_READ,       /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
		nullptr,                                 /* RTVとDSV用の設定 */
		IID_PPV_ARGS(m_pCB.GetAddressOf()));     /* アドレスを格納 */
	if (FAILED(hr)) {

		return false;
	}

	// 定数バッファのアドレス
	auto addressGPU = m_pCB->GetGPUVirtualAddress();               // GPUの仮想アドレスを取得
	auto handleCPU = pHeap->GetCPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（CPU）
	auto handleGPU = pHeap->GetGPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（GPU）

	// 定数バッファの先頭ポインタを計算（定数バッファのサイズ分をインクリメント）
	handleCPU.ptr += incrementSize * HeapCount;
	handleGPU.ptr += incrementSize * HeapCount;

	// 定数バッファビューの設定（定数バッファの情報を保存）
	m_HandleCPU           = handleCPU;          // 定数バッファの先頭ハンドル（CPU）
	m_HandleGPU           = handleGPU;          // 定数バッファの先頭ハンドル（GPU）
	m_Desc.BufferLocation = addressGPU;         // バッファの保存位置を指定
	m_Desc.SizeInBytes    = alignmentSize;      // 定数バッファのサイズ

	// 定数バッファビューの生成
	pDevice->CreateConstantBufferView(&m_Desc, handleCPU);


	//定数バッファ（Transform）のマッピングを行う
	hr = m_pCB->Map(0, nullptr, reinterpret_cast<void**>(&m_pMapBuf));  // 上で設定したサイズ分のGPUリソースへのポインタを取得
	if (FAILED(hr)) {

		std::cout << "定数バッファのマップエラー" << std::endl;
		return false;
	}

	return true;
}


void ConstantBuffer::Term() {

	// 定数バッファの破棄
	if (m_pCB != nullptr) {

		m_pCB->Unmap(0, nullptr);

		m_HandleCPU.ptr       = 0;
		m_HandleGPU.ptr       = 0;
		m_Desc.SizeInBytes    = 0;
		m_Desc.BufferLocation = 0;
		m_pMapBuf             = 0;
	}
	
	m_pCB.Reset();
}


D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleCPU() const {

	return m_HandleCPU;
}


D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleGPU() const {

	return m_HandleGPU;
}


D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetVirtualAddress() const {

	return m_Desc.BufferLocation;
}