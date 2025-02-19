
/****************************************************************
 * Include
 ****************************************************************/
#include "DescriptorManager.h"
#include <iostream>



DescriptorManager::DescriptorManager():
	m_pHeapRTV         (nullptr),
	m_pHeapDSV         (nullptr),
	m_pHeapCBV_SRV_UAV (nullptr),
	m_HandleRTV        (),
	m_HandleDSV        (),
	m_HeapCount        (0)
{ /* DO_NOTHING */ }


DescriptorManager::~DescriptorManager()
{ /* DO_NOTHING */ }



bool DescriptorManager::CreateRTV(ID3D12Device* pDevice, IDXGISwapChain3* pSwapChain) {

	if (pDevice == nullptr || pSwapChain == nullptr) {

		std::cout << "Error : DSV生成エラー" << std::endl;
		return false;
	}

	// ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;   /* RTVを設定 */
	desc.NumDescriptors = _FrameCount;                      /* ディスクリプタの数（フレームの数分）*/
	desc.NodeMask       = 0;                                /* 複数のGPUはない */
	desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* シェーダーからアクセス可能 */


	// ディスクリプタヒープの生成
	HRESULT hr = pDevice->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create RTV DescriptorHeap." << std::endl;
		return false;
	}


	// ディスクリプタヒープの先頭アドレスを取得
	auto handleCPU = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();

	// GPU固有のインクリメントサイズの取得
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


	// レンダーターゲットの生成
	for (auto i = 0u; i < _FrameCount; i++) {

		hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pRTB[i].GetAddressOf()));
		if (FAILED(hr)) {

			std::cout << "Error : Can't create RenderTargetBuffer." << std::endl;
			return false;
		}

		// レンダーターゲットビュー（RTV）の設定
		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  /* ディスプレイへの表示フォーマット */
		desc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;    /* ビューの次元 */
		desc.Texture2D.MipSlice   = 0;                                /* ミップレベル設定 */
		desc.Texture2D.PlaneSlice = 0;                                /* 平面スライス設定 */

		// レンダーターゲットビュー（RTV）の生成
		pDevice->CreateRenderTargetView(
			m_pRTB[i].Get(),
			&desc,
			handleCPU);

		m_HandleRTV[i] = handleCPU;      // レンダーターゲットビューの先頭アドレスを格納
		handleCPU.ptr += incrementSize;  // ポインタをインクリメント分動かす（次のRTVのアドレスまで動かす）
	}

	// 正常終了
	return true;
}


bool DescriptorManager::CreateDSV(ID3D12Device* pDevice, uint32_t Width, uint32_t Height) {

	if (pDevice == nullptr || Width == 0 || Height == 0) {

		std::cout << "Error : DSV生成エラー" << std::endl;
		return false;
	}

	// リソースのプロパティ設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type                 = D3D12_HEAP_TYPE_DEFAULT;          /* GPUアクセスの読み書きのみ・CPUはアクセス不可 */
	prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUのページプロパティ */
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプール設定 */
	prop.CreationNodeMask     = 1;                                /* GPUの数 */
	prop.VisibleNodeMask      = 1;                                /* GPU識別時に設定 */

	// リソースの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;       /* リソースの次元 */
	desc.Alignment          = 0;                                        /* バッファの区切り */
	desc.Width              = Width;                                    /* リソースの横幅 */
	desc.Height             = Height;                                   /* リソースの縦幅 */
	desc.DepthOrArraySize   = 1;                                        /* リソースの奥行 */
	desc.Format             = DXGI_FORMAT_D32_FLOAT;                    /* フォーマット */
	desc.MipLevels          = 1;                                        /* ミップレベル */
	desc.SampleDesc.Count   = 1;                                        /* マルチサンプリング（アンチエイリアス）設定 */
	desc.SampleDesc.Quality = 0;                                        /* マルチサンプリングのクオリティ設定 */
	desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;             /* レイアウト設定 */
	desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;  /* 深度ステンシルを指定 */


	// 深度ステンシルバッファのクリア値を設定
	D3D12_CLEAR_VALUE clear;
	clear.Format               = DXGI_FORMAT_D32_FLOAT;  /* 深度ステンシルバッファのフォーマット */
	clear.DepthStencil.Depth   = 1.0f;                   /* 深度値の初期値 */
	clear.DepthStencil.Stencil = 0;                      /* ステンシル値の初期値 */

	// 深度ステンシルバッファの生成
	HRESULT hr = pDevice->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clear,
		IID_PPV_ARGS(m_pDSB.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create Depth Stencil Buffer." << std::endl;
		return false;
	}


	// ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;                                /* ディスクリプタ（ビュー）の数 */
	heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   /* ディスクリプターの種類設定 */
	heapDesc.NodeMask       = 0;                                /* 複数GPUがある場合設定 */
	heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* シェーダーからは見れないようにする */

	hr = pDevice->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(m_pHeapDSV.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create DSV DescriptorHeap." << std::endl;
	}

	
	// DSV用ディスクリプターヒープの先頭ハンドルを取得
	auto handleCPU = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();

	// DSV用のインクリメントサイズを取得
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// 深度ステンシルバッファビューの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC DSVdesc = {};
	DSVdesc.Format             = DXGI_FORMAT_D32_FLOAT;          /* DSVのフォーマット */
	DSVdesc.Texture2D.MipSlice = 0;                              /* ミップスライス */
	DSVdesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;  /* ビューの次元 */
	DSVdesc.Flags              = D3D12_DSV_FLAG_NONE;            /* 読み取り・書き込み設定はしない */


	// DSVの生成
	pDevice->CreateDepthStencilView(
		m_pDSB.Get(),
		&DSVdesc,
		handleCPU);


	// DSV先頭アドレスを格納
	m_HandleDSV = handleCPU;

	// 正常終了
	return true;
}


bool DescriptorManager::CreateCBV(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap, ConstantBuffer* pCBV, size_t size) {

	if (pDevice == nullptr || pHeap == nullptr || pCBV == nullptr || size == 0) {

		std::cout << "CreateCBV Error : 引数が無効な値" << std::endl;
		return false;
	}

	// CBVの生成
	if (!pCBV->Init(pDevice, pHeap, size, m_HeapCount)) {

		std::cout << "Create Error : CBVの生成エラー" << std::endl;
		return false;
	}

	// 生成カウント
	AddCount();

	
	return true;
}


bool DescriptorManager::CreateSRV(
	ID3D12Device*                 pDevice, 
	ID3D12DescriptorHeap*         pHeap, 
	Texture*                      pSRV, 
	const wchar_t*                filename,
	DirectX::ResourceUploadBatch& batch)
{

	if (pDevice == nullptr || pHeap == nullptr) {

		std::cout << "Error : 引数の nullptrエラー" << std::endl;
		return false;
	}


	// SRVの作成
	if (!pSRV->Init(pDevice, pHeap, filename, batch, m_HeapCount)) {

		std::cout << "Create Error : SRVの生成エラー" << std::endl;
		return false;
	}

	// 生成カウント
	AddCount();


	return true;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::GetCPUHandle_RTV(const uint32_t FrameIndex) const{

	if (m_pHeapRTV == nullptr) {

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	return m_HandleRTV[FrameIndex];
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::GetCPUHandle_DSV() const{

	if (m_pHeapDSV == nullptr) {

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	return m_HandleDSV;
}


size_t DescriptorManager::GetCount() const {

	return m_HeapCount;
}


void DescriptorManager::Term() {

	TermRTV();
	TermDSV();
	m_HeapCount = 0;
}


void DescriptorManager::TermRTV() {

	// レンダーターゲットビューの破棄
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < _FrameCount; i++) {

		m_pRTB[i].Reset();
	}

}


void DescriptorManager::TermDSV() {

	// 深度ステンシルビューの破棄
	m_pHeapDSV.Reset();
	m_pDSB.Reset();
}