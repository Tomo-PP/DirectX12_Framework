
/****************************************************************
 * Include
 ****************************************************************/
#include "VertexBuffer.h"
#include "Mesh.h"
#include <iostream>


VertexBuffer::VertexBuffer() : 
	m_pVB  (nullptr),
	m_VBV  ()
{ /* DO_NOTHING */ }


VertexBuffer::~VertexBuffer()
{ /* DO_NOTHING */ }


bool VertexBuffer::Init(ID3D12Device* pDevice, size_t size, const Mesh* m_mesh) {

	if (pDevice == nullptr) {

		std::cout << "VertexBuffer Erro : 引数のnullptrエラー" << std::endl;
		return false;
	}

	// ヒーププロパティ（データをどう送るのかを記述）
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（今回は指定なし）*/
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */


	// バッファサイズを決定
	auto VertexSize = sizeof(MeshVertex) * m_mesh[0].Vertices.size();  /* MeshVertex（頂点情報）× 頂点情報の数 */
	auto vertices   = m_mesh[0].Vertices.data();                       /* マッピング用の頂点データを確保する（可変配列の先頭ポインタを取得）*/

	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	resourceDesc.Width              = VertexSize;                       /* 頂点情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
	resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
	resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/

	// 頂点バッファ（GPUリソース）を生成
	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,                                   /* ヒープの設定 */
		D3D12_HEAP_FLAG_NONE,                        /* ヒープのオプション */
		&resourceDesc,                               /* リソースの設定 */
		D3D12_RESOURCE_STATE_GENERIC_READ,           /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
		nullptr,                                     /* RTVとDSV用の設定 */
		IID_PPV_ARGS(m_pVB.GetAddressOf()));         /* アドレスを格納 */
	if (FAILED(hr)) {

		std::cout << "VertexBuffer Error : 頂点バッファの生成エラー" << std::endl;
		return false;
	}


	// 頂点バッファのマッピングを行う
	void* ptr = nullptr;
	hr = m_pVB->Map(0, nullptr, &ptr);  //　上で設定したサイズ分のGPUリソースへのポインタを取得
	if (FAILED(hr)) {

		return false;
	}


	// 頂点データをマッピング先に設定（GPUのメモリをコピー）
	memcpy(ptr, vertices, VertexSize);

	// マッピングの解除
	m_pVB->Unmap(0, nullptr);


	// 頂点バッファビューの設定（頂点バッファの描画コマンド用・GPUのアドレスやサイズなどを記憶しておく）
	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();                          /* 先ほどマップしたGPUの仮想アドレスを記憶 */
	m_VBV.SizeInBytes    = static_cast<UINT>(VertexSize);                          /* 頂点データ全体のサイズを記憶 */
	m_VBV.StrideInBytes  = static_cast<UINT>(sizeof(MeshVertex));                  /* １頂点辺りのサイズを記憶 */
}


void VertexBuffer::Term() {

	// 頂点バッファの破棄
	m_pVB.Reset();

	m_VBV.BufferLocation = 0;
	m_VBV.SizeInBytes    = 0;
	m_VBV.StrideInBytes  = 0;

}


D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVBV() const {

	return m_VBV;
}