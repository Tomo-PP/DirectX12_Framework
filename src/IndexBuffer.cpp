
/****************************************************************
 * Include
 ****************************************************************/
#include "IndexBuffer.h"
#include <iostream>


IndexBuffer::IndexBuffer() :
	m_pIB        (nullptr),
	m_IBV        (),
	m_IndexCount (0)
{ /* DO_NOTHING */ }


IndexBuffer::~IndexBuffer()
{ /* DO_NOTHING */ }


bool IndexBuffer::Init(ID3D12Device* pDevice, const Mesh* mesh) {

	// ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /*  */
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /*  */
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの設定 */
	heapProp.CreationNodeMask     = 1;                                /* GPUの識別マスク */
	heapProp.VisibleNodeMask      = 1;                                /*  */


	// インデックスバッファのサイズを決定
	m_IndexCount = mesh->Indices.size();
	auto IndexSize = sizeof(uint32_t) * m_IndexCount;  /* インデックスはuint32_t型 × インデックスの数 */
	auto indices   = mesh->Indices.data();             /* マッピング用インデックスデータの先頭ポインタを取得 */


	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	resourceDesc.Width              = IndexSize;                        /* インデックス情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
	resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
	resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（今回はバッファなので *UNKNOWN。テクスチャの場合はピクセルフォーマットを指定）*/
	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/


	// リソースの生成
	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pIB.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "インデックスバッファのエラー" << std::endl;
		return false;
	}


	// マッピングを行う
	void* ptr = nullptr;
	hr = m_pIB->Map(0, nullptr, &ptr);
	if (FAILED(hr)) {

		return false;
	}

	// インデックスデータをGPUメモリにコピーする
	memcpy(ptr, indices, IndexSize);


	// マッピングの解除
	m_pIB->Unmap(0, nullptr);


	// インデックスバッファビューの設定
	m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();    /* インデックスバッファのGPUメモリ */
	m_IBV.Format         = DXGI_FORMAT_R32_UINT;             /* フォーマット（ポリゴン数が多い場合 *R32_UINT ポリゴン数が少ない場合 *R16_UINT） */
	m_IBV.SizeInBytes    = static_cast<UINT>(IndexSize);     /* インデックスデータのデータサイズ（バイト）*/

	// 正常終了
	return true;
}


void IndexBuffer::Term() {

	// インデックスバッファの破棄
	m_pIB.Reset();

	m_IBV.BufferLocation = 0;
	m_IBV.Format         = DXGI_FORMAT_UNKNOWN;
	m_IBV.SizeInBytes    = 0;
	m_IndexCount         = 0;
}


D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetIBV() {

	return m_IBV;
}

uint32_t IndexBuffer::GetIndexNum() const {

	return m_IndexCount;
}