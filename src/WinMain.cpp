#include "WinMain.h"
#include <iostream>
#include <string>

App::App(uint32_t width, uint32_t height, LPCWSTR title):
	m_hInst       (nullptr),
	m_hWnd        (nullptr),
	m_Width       (width),
	m_Height      (height),
	m_windowTitle (title),
	m_pDevice     (nullptr),
	m_pCmdQueue   (nullptr),
	m_pSwapChain  (nullptr),
	m_pCmdList    (nullptr),
	m_pHeapRTV    (nullptr),
	m_pFence      (nullptr),
	m_pVB         (nullptr),
	m_pPSO        (nullptr),
	m_FrameIndex  (0)
{
	for (auto i = 0u; i < FrameCount; i++) {

		m_pCmdAllocator[i] = nullptr;
		m_pColorBuffer[i]  = nullptr;
		m_FenceCounter[i]  = 0;
	}
}

App::~App()
{ }


void App::Run() {

	// メインループ
	if (InitApp()) {
		MainLoop();
	}

	// アプリの破棄
	TermApp();
	
}


bool App::InitApp() {

	// ウィンドウの初期化
	if (!InitWindow()) {

		return false;
	}

	// Direct3Dの初期化
	if (!InitDirect3D()) {

		return false;
	}

	// 描画関連の初期化
	if (!OnInit()) {

		return false;
	}


	//正常終了
	return true;
}


bool App::InitWindow() {

	// インスタンスハンドルの取得（このプログラム自体のインスタンス）
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		std::cout << "Failed to Get Instance Handle." << std::endl;
		return false;
	}

	// ウィンドウの設定
	WNDCLASSEX windowClass = {};
	windowClass.cbSize           = sizeof(WNDCLASSEX);                  /* 構造体のサイズ */
	windowClass.style            = CS_HREDRAW | CS_VREDRAW;             /* ウィンドウクラスのスタイル（ウィンドウの枠線などの設定）*/
	windowClass.lpfnWndProc      = WndProc;                             /* ウィンドウプロシージャ */
	windowClass.hIcon            = LoadIcon(hInst, IDI_APPLICATION);    /* ウィンドウのアイコンのハンドル設定 */
	windowClass.hCursor          = LoadCursor(hInst, IDC_ARROW);        /* ウィンドウのマウスカーソル設定 */
	windowClass.hbrBackground    = GetSysColorBrush(COLOR_BACKGROUND);  /* ウィンドウの背景を描画するブラシへのハンドル */
	windowClass.lpszMenuName     = nullptr;                             /* メニューバー表示の際に設定（今回は使用しないので nullptr ）*/
	windowClass.lpszClassName    = m_windowTitle;                       /* ウィンドウのタイトル */
	windowClass.hIconSm          = LoadIcon(hInst, IDI_APPLICATION);    /* ウィンドウのアイコンの設定 */

	// ウィンドウの登録
	if (!RegisterClassEx(&windowClass)) {
		return false;
	}


	// インスタンスハンドルを設定（ポインタの受け渡し）
	m_hInst = hInst;


	// ウィンドウサイズの設定（左と上がそれぞれ０の基準）
	RECT rect = {};
	rect.right  = m_Width;   /* 横幅 */
	rect.bottom = m_Height;  /* 高さ */


	// ウィンドウサイズを調整
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rect, style, FALSE);


	// ウィンドウを生成
	m_hWnd = CreateWindowEx(
		0,                       /* 拡張ウィンドウの設定（今回は使用しない）*/
		m_windowTitle,           /* ウィンドウクラス名の設定 */
		m_windowTitle,           /* ウィンドウのタイトルバーに表示される名前の設定 */
		style,                   /* ウィンドウのスタイル設定（メニューバーなどの設定）*/
		CW_USEDEFAULT,           /* ウィンドウの初期 x座標 */
		CW_USEDEFAULT,           /* ウィンドウの初期 y座標 */
		rect.right - rect.left,  /* ウィンドウ幅 */
		rect.bottom - rect.top,  /* ウィンドウの高さ */
		nullptr,                 /* 作成するウィンドウの親またはオーナーウィンドウのハンドルを指定 */
		nullptr,                 /* ウィンドウスタイルに応じた子ウィンドウIDの指定（今回は使用しない）*/
		m_hInst,                 /* ウィンドウに関連付けられているインスタンスのハンドルを設定 */
		nullptr);                /* 生成するウィンドウに渡す任意のパラメーターを指定（今回は使用しない）*/

	if (m_hWnd == nullptr) {
		return false;
	}


	// ウィンドウを表示
	ShowWindow(m_hWnd, SW_SHOWNORMAL);


	// ウィンドウの更新
	UpdateWindow(m_hWnd);


	// ウィンドウにフォーカス
	SetFocus(m_hWnd);


	// 正常終了
	return true;
}


bool App::InitDirect3D() {

	// デバッグレイヤーの追加
#if defined (DEBUG) || defined (_DEBUG)
	ComPtr<ID3D12Debug> debug;
	HRESULT Hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
	if (SUCCEEDED(Hr)) {

		debug->EnableDebugLayer();
	}
#endif

	// デバイスの生成
	HRESULT hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf()));
	if (FAILED(hr)) {

		return false;
	}


	// コマンドキューの生成
	{
		D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
		CmdQueueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;       // GPUにコマンドキューを直接実行
		CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;  // 優先度はデフォルト
		CmdQueueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;        // コマンドキュー特性（今回はデフォルト）
		CmdQueueDesc.NodeMask = 0;                                    // GPUは 1台なので 0

		hr = m_pDevice->CreateCommandQueue(
			&CmdQueueDesc,
			IID_PPV_ARGS(m_pCmdQueue.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}
	}


	// スワップチェインの生成
	{
		// DXGIファクトリーの生成
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) {

			return false;
		}

		// スワップチェインの設定
		DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
		SwapChainDesc.BufferDesc.Width                   = m_Width;                                 /* 解像度の横幅 */
		SwapChainDesc.BufferDesc.Height                  = m_Height;                                /* 解像度の縦幅 */
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;                                       /* リフレッシュレートの分母 */
		SwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;                                      /* リフレッシュレートの分子 */
		SwapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;           /* スケーリングはなし */
		SwapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;    /* 走査線の順序は指定なし */
		SwapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;              /* GPUにはRGBA8ビットを渡す（0から1に正規化）*/
		SwapChainDesc.SampleDesc.Count                   = 1;                                       /* ピクセル単位のマルチサンプリング数（今回は必要ないので１） */
		SwapChainDesc.SampleDesc.Quality                 = 0;                                       /* 画像の品質レベルを設定 */
		SwapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;         /* バックバッファの使用方法を指定 */
		SwapChainDesc.BufferCount                        = FrameCount;                              /* バッファの数（ダブルバッファリングのため２） */
		SwapChainDesc.OutputWindow                       = m_hWnd;                                  /* 出力のウィンドウハンドルを指定 */
		SwapChainDesc.Windowed                           = TRUE;                                    /* スワップチェインがウィンドウモードで動くかの設定 */
		SwapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;           /* スワップチェイン実行時の動き（今回は変更後に破棄）*/
		SwapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  /* スワップチェインの動作オプション */
	
		// スワップチェインの生成
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(
			m_pCmdQueue.Get(),
			&SwapChainDesc,
			&pSwapChain);
		if (FAILED(hr)) {

			SafeRelease(pFactory);
			return false;
		}

		// バックバッファ番号の取得のためにIDXGIFatory3 を取得
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.GetAddressOf()));
		if (FAILED(hr)) {

			SafeRelease(pSwapChain);
			SafeRelease(pFactory);

			return false;
		}

		// バックバッファの番号を取得
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();


		// 必要ないオブジェクトの解放
		SafeRelease(pSwapChain);
		SafeRelease(pFactory);
	}


	// コマンドアロケーターの生成（フレームカウント数分生成する）
	{
		for (auto i = 0u; i < FrameCount; i++) {

			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));
			if (FAILED(hr)) {

				return false;
			}
		}
	}


	// コマンドリストの生成
	{
		hr = m_pDevice->CreateCommandList(
			0,                                      /* ノードマスクの設定（今回はGPUの数が 1つなので 0） */
			D3D12_COMMAND_LIST_TYPE_DIRECT,         /* 作成するコマンドリストのタイプを設定。（今回はコマンドキューに直接登録するため *DIRECT） */
			m_pCmdAllocator[m_FrameIndex].Get(),    /* コマンドアロケーターの設定 */
			nullptr,                                /* パイプラインステートの設定（この後に明示的に設定するため nullptr） */
			IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}
	}


	// レンダーターゲットビューの生成（フレーム数分生成、バックバッファ用のビュー）
	{
		// ディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = FrameCount;                       /* ヒープ内のディスクリプタ数を指定（ダブルバッファリングのため２）*/
		heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;   /* ディスクリプタヒープの種類を指定（今回はレンダーターゲットビュー）*/
		heapDesc.NodeMask       = 0;                                /* 複数のGPUがある場合指定（今回は、1つなので 0）*/
		heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* 参照可能かのフラグ（今回はRTV直で参照不可の NONEを指定）*/

		// ディスクリプタヒープの生成
		hr = m_pDevice->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
		if (FAILED(hr)) {
			return false;
		}


		// ディスクリプタヒープの先頭アドレスを取得
		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();

		// GPUによってアドレスのメモリ量が変わるので、GPU固有の値を取得したうえでアドレス計算を行う必要がある
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		// レンダーターゲットの生成（フレームカウント数分生成）
		for (auto i = 0u; i < FrameCount; i++) {

			// スワップチェインで確保したバッファのアドレスを割り当てる
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffer[i].GetAddressOf()));
			if (FAILED(hr)) {

				return false;
			}

			// レンダーターゲットビューの設定
			D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
			RTVDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;   /* ディスプレイへの表示フォーマット */
			RTVDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;     /* どのようにレンダーターゲットビューにアクセスするかの次元を指定 */
			RTVDesc.Texture2D.MipSlice   = 0;                                 /* ミップレベルの設定 */
			RTVDesc.Texture2D.PlaneSlice = 0;                                 /* 平面スライスの番号を指定 */


			// レンダーターゲットの生成
			m_pDevice->CreateRenderTargetView(
				m_pColorBuffer[i].Get(),
				&RTVDesc,
				handle);

			m_HandleRTV[i] = handle;      // ディスクリプタヒープの先頭アドレスを保存（レンダーターゲットを扱えるようにするため）
			handle.ptr += incrementSize;  // 次のポインタのアドレスを計算
		}
	}


	// フェンスの生成
	{
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,          /* フェンスの共有の設定（今回は共有しない）*/
			IID_PPV_ARGS(m_pFence.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}
	}


	// イベントの生成
	{
		m_FenceEvent = CreateEvent(
			nullptr,                 /* 子プロセスが取得したハンドルを継承できるか決定する構造体へのポインタを指定 */
			FALSE,                   /* 今回は自動のリセットオブジェクトを設定 */
			FALSE,                   /* イベントオブジェクトの初期状態（今回は非シグナル状態 FALSE を指定） */
			nullptr);                /* イベントオブジェクトの名前の登録（今回は使用しない） */
		if (m_FenceEvent == nullptr) {

			return false;
		}
	}


	// コマンドリストを閉じる
	m_pCmdList->Close();


	// 正常終了
	return true;
}


bool App::OnInit() {

	// 頂点バッファの生成
	{
		// 頂点情報（ 四角形：｛ 座標, uv座標 ｝ ）
		DirectX::VertexPositionTexture vertices[] = {
			DirectX::VertexPositionTexture( DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2( 0.0f, 0.0f) ),
			DirectX::VertexPositionTexture( DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2( 1.0f, 0.0f) ),
			DirectX::VertexPositionTexture( DirectX::XMFLOAT3( 1.0f,-1.0f, 0.0f), DirectX::XMFLOAT2( 1.0f, 1.0f) ),
			DirectX::VertexPositionTexture( DirectX::XMFLOAT3(-1.0f,-1.0f, 0.0f), DirectX::XMFLOAT2( 0.0f, 1.0f) )
		};



		// ヒーププロパティ（データをどう送るのかを記述）
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
		heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（今回は指定なし）*/
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
		heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
		heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */


		// リソースの設定
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
		resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
		resourceDesc.Width              = sizeof(vertices);                 /* 頂点情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
		resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
		resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
		resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
		resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
		resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
		resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/

		// 頂点バッファ（GPUリソース）を生成
		HRESULT hr = m_pDevice->CreateCommittedResource(
			&heapProp,                                   /* ヒープの設定 */
			D3D12_HEAP_FLAG_NONE,                        /* ヒープのオプション */
			&resourceDesc,                               /* リソースの設定 */
			D3D12_RESOURCE_STATE_GENERIC_READ,           /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
			nullptr,                                     /* RTVとDSV用の設定 */
			IID_PPV_ARGS(m_pVB.GetAddressOf()));         /* アドレスを格納 */
		if (FAILED(hr)) {

			return false;
		}


		// 頂点バッファのマッピングを行う
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);  //　上で設定したサイズ分のGPUリソースへのポインタを取得
		if (FAILED(hr)) {

			return false;
		}


		// 頂点データをマッピング先に設定（GPUのメモリをコピー）
		memcpy(ptr, vertices, sizeof(vertices));

		// マッピングの解除
		m_pVB->Unmap(0, nullptr);


		// 頂点バッファビューの設定（頂点バッファの描画コマンド用・GPUのアドレスやサイズなどを記憶しておく）
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();                                /* 先ほどマップしたGPUの仮想アドレスを記憶 */
		m_VBV.SizeInBytes    = static_cast<UINT>(sizeof(vertices));                          /* 頂点データ全体のサイズを記憶 */
		m_VBV.StrideInBytes  = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture));    /* １頂点辺りのサイズを記憶 */
	}




	// インデックスバッファの生成
	{
		uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

		// ヒーププロパティ
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /*  */
		heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /*  */
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /*  */
		heapProp.CreationNodeMask     = 1;                                /*  */
		heapProp.VisibleNodeMask      = 1;                                /*  */


		// リソースの設定
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
		resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
		resourceDesc.Width              = sizeof(indices);                  /* インデックス情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
		resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
		resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
		resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
		resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（今回はバッファなので *UNKNOWN。テクスチャの場合はピクセルフォーマットを指定）*/
		resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
		resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/
		

		// リソースの生成
		HRESULT hr = m_pDevice->CreateCommittedResource(
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
		memcpy(ptr, indices, sizeof(indices));


		// マッピングの解除
		m_pIB->Unmap(0, nullptr);


		// インデックスバッファビューの設定
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();  /* インデックスバッファのGPUメモリ */
		m_IBV.Format         = DXGI_FORMAT_R32_UINT;           /* フォーマット（ポリゴン数が多い場合 *R32_UINT ポリゴン数が少ない場合 *R16_UINT） */
		m_IBV.SizeInBytes    = sizeof(indices);                /* インデックスデータのデータサイズ（バイト）*/
	}



	// CBV / SRV / UAV用のディスクリプタヒープの生成
	{

		// ディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     /* 定数バッファを含んだフラグを指定 */
		heapDesc.NumDescriptors = 2 * FrameCount;                             /* バックバッファの数×描画するデータの数 */
		heapDesc.NodeMask       = 0;                                          /* GPUは１つなので０を指定 */
		heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  /* シェーダー側から参照できるようにする */

		// ディスクリプタヒープの生成
		HRESULT hr = m_pDevice->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(m_pHeapCBV_SRV_UAV.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}
	}
	


	// 定数バッファの生成（座標行列などをシェーダーに渡すバッファ）
	{

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
		resourceDesc.Width              = sizeof(Transform);                /* データサイズ：256 Byte（256 Byteを超える場合 512 Byte）*/
		resourceDesc.Height             = 1;                                /* データの縦のサイズ（バッファなので１）*/
		resourceDesc.DepthOrArraySize   = 1;                                /* データの奥行（バッファなので１）*/
		resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
		resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
		resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
		resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定（今回は使わないので０） */
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* 始まりから終わりまで連続したバッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する） */


		// ディスクリプタヒープ内のデータについて、次のデータに移動するためのインクリメントサイズを取得
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


		// バックバッファの数分の定数バッファを生成（GPUリソース）
		for (auto i = 0u; i < FrameCount; i++) {

			HRESULT hr = m_pDevice->CreateCommittedResource(
				&heapProp,                               /* ヒープの設定 */
				D3D12_HEAP_FLAG_NONE,                    /* ヒープのオプション */
				&resourceDesc,                           /* リソースの設定 */
				D3D12_RESOURCE_STATE_GENERIC_READ,       /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
				nullptr,                                 /* RTVとDSV用の設定 */
				IID_PPV_ARGS(m_pCB[i].GetAddressOf()));  /* アドレスを格納 */
			if (FAILED(hr)) {

				return false;
			}

			// 定数バッファのアドレス
			auto addressGPU = m_pCB[i]->GetGPUVirtualAddress();                         // GPUの仮想アドレスを取得
			auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（CPU）
			auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（GPU）

			// 定数バッファの先頭ポインタを計算（定数バッファのサイズ分をインクリメント）
			handleCPU.ptr += incrementSize * i;
			handleGPU.ptr += incrementSize * i;

			// 定数バッファビューの設定（定数バッファの情報を保存）
			m_CBV[i].HandleCPU = handleCPU;          // 定数バッファの先頭ハンドル（CPU）
			m_CBV[i].HandleGPU = handleGPU;          // 定数バッファの先頭ハンドル（GPU）
			m_CBV[i].CBVDesc.BufferLocation = addressGPU;         // バッファの保存位置を指定
			m_CBV[i].CBVDesc.SizeInBytes = sizeof(Transform);  // 定数バッファのサイズ

			// 定数バッファビューの生成
			m_pDevice->CreateConstantBufferView(&m_CBV[i].CBVDesc, handleCPU);


			//定数バッファ（Transform）のマッピングを行う
			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));  // 上で設定したサイズ分のGPUリソースへのポインタを取得
			if (FAILED(hr)) {
				std::cout << "定数バッファのマップエラー" << std::endl;
				return false;
			}

			// マッピングした行列を設定する
			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);     /* カメラ座標 */
			auto targetPos = DirectX::XMVectorZero();                       /* 注視点座標（原点）*/
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);     /* カメラの高さ */

			auto fovY = DirectX::XMConvertToRadians(37.5f);                           /* カメラの Y軸に対する画角 */
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height); /* 高さに対する幅の割合 */

			// 変換行列の設定
			m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();                                          /* ワールド行列・Identityは単位行列 */
			m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);                  /* カメラ行列 */
			m_CBV[i].pBuffer->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);  /* 射影行列 */
		}
	}



	// 深度ステンシルビューの生成（RTVの生成の流れとほぼ同じ）
	{
		// リソースのプロパティ設定
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type                 = D3D12_HEAP_TYPE_DEFAULT;          /* デフォルトに設定（CPUアクセス不可・GPUアクセスは読み書き可能）*/
		heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUのページプロパティ設定 */
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* 今回はメモリプールはなし */
		heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
		heapProp.VisibleNodeMask      = 1;                                /* GPU識別の際に設定 */

		// リソースの設定
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;       /* 今回は縦横が存在するので *TEXTURE2D */
		resourceDesc.Alignment          = 0;                                        /* バッファの区切りの設定（今回は１つなので区切りはいらない）*/
		resourceDesc.Width              = m_Width;                                  /* 画面の幅分 */
		resourceDesc.Height             = m_Height;                                 /* 画面の高さ分 */
		resourceDesc.DepthOrArraySize   = 1;                                        /* 奥行は今回１ */
		resourceDesc.MipLevels          = 1;                                        /* ミップレベルの設定 */
		resourceDesc.Format             = DXGI_FORMAT_D32_FLOAT;                    /* 奥行専用のフォーマット（３２ビット浮動小数）*/
		resourceDesc.SampleDesc.Count   = 1;                                        /* マルチサンプリング（アンチエイリアス）の設定（今回は使用しない）*/
		resourceDesc.SampleDesc.Quality = 0;                                        /* マルチサンプリングのクオリティ */
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;             /*  */
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;  /* 深度ステンシルなので *DEPTH_STENCIL を指定*/


		// 深度ステンシルバッファのクリア値（初期化値）を設定できる
		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format               = DXGI_FORMAT_D32_FLOAT;  /* フォーマットは深度専用の D */
		clearValue.DepthStencil.Depth   = 1.0f;                   /* 深度の初期化値 */
		clearValue.DepthStencil.Stencil = 0;                      /* ステンシル値の初期化値 */


		// リソースの生成
		HRESULT hr = m_pDevice->CreateCommittedResource(
			&heapProp,                                /* ヒープの設定 */
			D3D12_HEAP_FLAG_NONE,                     /* オプション設定 */
			&resourceDesc,                            /* リソース設定 */
			D3D12_RESOURCE_STATE_DEPTH_WRITE,         /* リソースの初期は書き込まれる */
			&clearValue,                              /* 初期化の際の値 */
			IID_PPV_ARGS(m_pDSB.GetAddressOf()));
		if (FAILED(hr)) {

			std::cout << "深度バッファビューのリソース取得エラー" << std::endl;
			return false;
		}


		// 深度バッファビュー用ディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;                                /* 深度ステンシルはGPU実行最中のみ使用されるので、ダブルバッファ化をしなくてよい */
		heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   /* 深度ステンシルなので *DSVを指定 */
		heapDesc.NodeMask       = 0;                                /* 複数のGPUがある場合指定（今回は、1つなので 0）*/
		heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* 参照可能かのフラグ */

		hr = m_pDevice->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(m_pHeapDSV.GetAddressOf()));
		if (FAILED(hr)) {
			std::cout << "深度ステンシルビュー用のディスクリプタの生成エラー" << std::endl;
			return false;
		}

		
		// 深度ステンシルビューの設定
		auto handle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();   /* ディスクリプタヒープの初めポインタを取得 */
		auto incrementSize = 
			m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);  /* GPU内の深度ステンシルビューのインクリメントサイズ（種類によって違う）*/

		D3D12_DEPTH_STENCIL_VIEW_DESC DSVdesc = {};
		DSVdesc.Format             = DXGI_FORMAT_D32_FLOAT;          /* 深度ステンシルビューのフォーマット（DSバッファと同じ）*/
		DSVdesc.Texture2D.MipSlice = 0;                              /* ミップスライスの設定 */
		DSVdesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;  /* ビューの次元（縦横の二次元テクスチャ）*/
		DSVdesc.Flags              = D3D12_DSV_FLAG_NONE;            /* 読み取り・書き込み専用は設定しない */

		// 深度ステンシルビューの生成
		m_pDevice->CreateDepthStencilView(
			m_pDSB.Get(),
			&DSVdesc,
			handle);

		m_HandleDSV = handle;   /* ディスクリプタハンドルの設定（ヒープの先頭ポインタ）*/
	}


	// テクスチャの生成
	{
		// ファイルパスの検索
		std::wstring texturePath;
		if (!SearchFilePath(L"texture/house.dds", texturePath)) {

			return false;
		}


		// 画像データの読み取り処理
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());
		batch.Begin();                                        // 内部処理でコマンドアロケーターとコマンドリストが生成される

		// リソースの生成（この関数で一気にリソースを生成できる。リソース設定からしなくてもよい）
		HRESULT hr = DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(),                     /* デバイスを渡す */
			batch,                               /*  */
			texturePath.c_str(),                 /* テクスチャまでのパス（ワイド文字をマルチバイトに変換して渡す必要あり）*/
			m_Texture.pResource.GetAddressOf(),  /* テクスチャのアドレスを取得 */
			true);                               /*  */
		if (FAILED(hr)) {

			return false;
		}


		// コマンドの実行
		auto future = batch.End(m_pCmdQueue.Get());

		// コマンド完了まで待機
		future.wait();

		// インクリメントサイズを取得（シェーダー用（SRV）のメモリのインクリメントサイズ）
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// CPUディスクリプタハンドルとGPUディスクリプタハンドルをディスクリプタヒープから取得
		auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
		auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

		// テクスチャにディスクリプタハンドルを割り当てる（コンスタントバッファ分のディスクリプタヒープを飛ばす必要あり）
		handleCPU.ptr += incrementSize * 2;
		handleGPU.ptr += incrementSize * 2;

		m_Texture.HandleCPU = handleCPU;
		m_Texture.HandleGPU = handleGPU;

		// テクスチャの構成を取得（上のライブラリで生成された構成を読み込む）
		auto textureDesc = m_Texture.pResource->GetDesc();

		// シェーダリソースビューの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
		SRVDesc.Format                        = textureDesc.Format;                        /*  */
		SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
		SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
		SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
		SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
		SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

		// シェーダーリソースビューの生成
		m_pDevice->CreateShaderResourceView(
			m_Texture.pResource.Get(),         /*  */
			&SRVDesc,                          /*  */
			handleCPU);                        /*  */
	}



	// ルートシグネチャ生成（シェーダー内で使用するリソースの扱い方を決める）
	{

		// ルートシグネチャのレイアウトオプションの設定（論理和で指定・使用しないものを記述）
		auto flagLayout = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;     /* ハルシェーダーのルートシグネチャへのアクセスを拒否 */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;   /* ドメインシェーダーのルートシグネチャへのアクセスを拒否 */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; /* ジオメトリーシェーダーのルートシグネチャへのアクセスを拒否 */

		
		// ルートパラメーターの設定（定数バッファ用・CBV）
		D3D12_ROOT_PARAMETER rootParam[2] = {};
		rootParam[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;  /* ルートパラメーターのタイプは定数バッファ（CBV）*/
		rootParam[0].Descriptor.ShaderRegister = 0;                              /*  */
		rootParam[0].Descriptor.RegisterSpace  = 0;                              /* 今回は使わない */
		rootParam[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX; /* 頂点シェーダーから参照できるようにする */

		// ディスクリプターテーブルの範囲を設定
		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  /* ディスクリプタの種類を設定 */
		range.NumDescriptors                    = 1;                                /* ディスクリプターの数（今回はSRVの一つだけ） */
		range.BaseShaderRegister                = 0;                                /*  */
		range.RegisterSpace                     = 0;                                /*  */
		range.OffsetInDescriptorsFromTableStart = 0;                                /*  */


		// ルートパラメーターの設定（テクスチャ用・SRV）
		rootParam[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  /* ルートパラメーターのタイプ（ディスクリプターテーブル）*/
		rootParam[1].DescriptorTable.NumDescriptorRanges = 1;                                           /*  */
		rootParam[1].DescriptorTable.pDescriptorRanges   = &range;                                      /* 設定したディスクリプタレンジを指定 */
		rootParam[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               /* ピクセルシェーダーで使用できるようにする */


		// スタティックサンプラーの設定
		D3D12_STATIC_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;              /* テクスチャをサンプリングする際のフィルタリング方法 */
		SamplerDesc.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* U成分に対するテクスチャアドレッシング */
		SamplerDesc.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* V成分に対するテクスチャアドレッシング */
		SamplerDesc.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* W成分に対するテクスチャアドレッシング */
		SamplerDesc.MipLODBias       = D3D12_DEFAULT_MIP_LOD_BIAS;                   /* 計算されたミップレベルのオフセット（計算後３：バイアス：２の場合、適応されるのは５） */
		SamplerDesc.MaxAnisotropy    = 1;                                            /* フィルタリングがANISOTROPICの場合のみ有効 */
		SamplerDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;                  /* 既存のサンプルデータに対してサンプルされたデータの比較設定（今回はなし） */
		SamplerDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  /* アドレッシングモードが *_BORDERの場合の境界線の色を設定 */
		SamplerDesc.MinLOD           = -D3D12_FLOAT32_MAX;                           /* ミップマップ範囲の下限値を設定 */
		SamplerDesc.MaxLOD           = +D3D12_FLOAT32_MAX;                           /* ミップマップ範囲の上限値を設定 */
		SamplerDesc.ShaderRegister   = 0;                                            /* HLSLのバインディングに対応するレジスタを設定 */
		SamplerDesc.RegisterSpace    = 0;                                            /* レジスタ空間の設定 */
		SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                /* ピクセルシェーダーで使用できるように設定 */


		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.NumParameters     = 2;             /* ルートパラメーターの数 */
		rootSigDesc.NumStaticSamplers = 1;             /* スタティックサンプラーの数 */
		rootSigDesc.pParameters       = rootParam;     /* 定義したルートパラメーターを指定（配列の先頭アドレス）*/
		rootSigDesc.pStaticSamplers   = &SamplerDesc;  /* 先ほど設定したスタティックサンプラーを指定 */
		rootSigDesc.Flags             = flagLayout;    /* 指定したものがルートシグネチャをアクセスすることを拒否するフラグ */

		// ルートシグネチャのシリアライズ（バイト列に変換）
		ComPtr<ID3DBlob> pBlob;                     /* ルートシグネチャをバイト変換したものを保存 */
		ComPtr<ID3DBlob> pErrorBlob;                /* シリアライズに失敗時のエラー内容 */

		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,                           /* 先ほど設定したルートシグネチャを指定 */
			D3D_ROOT_SIGNATURE_VERSION_1_0,         /* ルートシグネチャのバージョン指定 */
			pBlob.GetAddressOf(),                   /* シリアライズ化したルートシグネチャを保存 */
			pErrorBlob.GetAddressOf());             /* シリアライズに失敗した場合にエラー内容が書き込まれる */
		if (FAILED(hr)) {

			std::cout << "ルートシグネチャシリアルエラー" << std::endl;
			return false;
		}

		// ルートシグネチャの生成
		hr = m_pDevice->CreateRootSignature(
			0,                                               /* ノードマスクの設定（GPUが１つだけなので０） */
			pBlob->GetBufferPointer(),                       /* シリアライズしたデータへのポインタ */
			pBlob->GetBufferSize(),                          /* シリアライズしたデータのサイズ */
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf()));
		if (FAILED(hr)) {
			std::cout << "ルートシグネチャ生成エラー" << std::endl;
			return false;
		}
	}


	// パイプラインステートの生成
	{

		// 入力レイアウトの設定
		D3D12_INPUT_ELEMENT_DESC InputElement[2];  // 入力のデータ数分サイズを設定（今回は位置座標・色）

		// 頂点情報の入力設定（POSITION）
		InputElement[0].SemanticName         = "POSITION";                                  /* 頂点シェーダーで定義したセマンティック名を指定 */
		InputElement[0].SemanticIndex        = 0;                                           /* セマンティックインデックスを指定（セマンティック名が同じ場合設定）*/
		InputElement[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;              /* 入力のフォーマットを指定 */
		InputElement[0].InputSlot            = 0;                                           /* 入力スロット番号の指定（頂点バッファを複数扱わないので０）*/
		InputElement[0].AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT;                /* オフセットをバイト単位で指定（今回はデータが連続しているので *ALIGNEDを指定）*/
		InputElement[0].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;  /* 頂点ごとのデータとして扱うように指定 */
		InputElement[0].InstanceDataStepRate = 0;                                           /* 頂点ごとにデータを扱うので０（インスタンスごとの場合設定） */

		// 色情報の入力設定（TEXCOORD）
		InputElement[1].SemanticName         = "TEXCOORD";                                  /* 頂点シェーダーで定義したセマンティック名を指定 */
		InputElement[1].SemanticIndex        = 0;                                           /* セマンティックインデックスを指定（セマンティック名が同じ場合設定）*/
		InputElement[1].Format               = DXGI_FORMAT_R32G32_FLOAT;                    /* 入力のフォーマットを指定 */
		InputElement[1].InputSlot            = 0;                                           /* 入力スロット番号の指定（頂点バッファを複数扱わないので０） */
		InputElement[1].AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT;                /* オフセットをバイト単位で指定（今回はデータが連続しているので *ALIGNEDを指定）*/
		InputElement[1].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;  /* 頂点ごとのデータとして扱うように指定 */
		InputElement[1].InstanceDataStepRate = 0;                                           /* 頂点ごとにデータを扱うので０（インスタンスごとの場合設定）*/



		// ラスタライザーステートの設定（ポリゴンデータからピクセルデータへとラスタライズを行う際の状態設定）
		D3D12_RASTERIZER_DESC RSdesc = {};
		RSdesc.FillMode              = D3D12_FILL_MODE_SOLID;                      /* SOLID：中身も塗る・WIREFRAME：ワイヤーのみ（頂点間を結ぶ線のみ）*/
		RSdesc.CullMode              = D3D12_CULL_MODE_NONE;                       /* カリング処理の設定（指定した方向に属する面の描画をしないようにする設定） */
		RSdesc.FrontCounterClockwise = FALSE;                                      /* 全面が時計回りか反時計回りかを指定 */
		RSdesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;                   /* 与えられtピクセルに加算する深度値を設定。（奥行を調整するためのもの。今回は使用しない） */
		RSdesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;             /* ピクセル最大深度バイアス値を設定（今回は使用しない） */
		RSdesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;      /* ピクセルの勾配に応じた深度バイアス値をスケールｓるスカラー値を設定（今回は使用しない） */
		RSdesc.DepthClipEnable       = FALSE;                                      /* 距離に基くクリッピングを有効にするかを指定 */
		RSdesc.MultisampleEnable     = FALSE;                                      /* マルチサンプリング（アンチエイリアシング）を有効にするかを指定 */
		RSdesc.AntialiasedLineEnable = FALSE;                                      /* 線のアンチエイリアシングの設定 */
		RSdesc.ForcedSampleCount     = 0;                                          /* アンオーダーアクセスビューの描画またはラスタライザの間、サンプル数を強制的に固定値にする（今回は強制しないので０） */
		RSdesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;  /* ラスタライザーで少しでもそのピクセルが線にかかっていたらそのピクセルもラスタ化する */


		// ブレンドステートの設定（入力要素・演算要素・出力要素の３つをどのように混合するかを設定・半透明の処理ができる）

		// まずレンダーターゲットのブレンドを設定
		D3D12_RENDER_TARGET_BLEND_DESC RTVBlendDesc = {
			FALSE,                          /* ブレンディングを有効にするか */
			FALSE,                          /* 論理演算を有効にするか */
			D3D12_BLEND_ONE,                /* RGB  ピクセルシェーダーから出力されたRGB値に対するブレンドオプションを指定 */
			D3D12_BLEND_ZERO,               /* RGB  現在のレンダーターゲットのRGB値に対して実行するブレンドオプションを指定 */
			D3D12_BLEND_OP_ADD,             /* 結合 上の二つをどのように結合するかを定義するオプション */
			D3D12_BLEND_ONE,                /* α ピクセルシェーダーから出力されたアルファ値に対して実行するオプションを指定 */
			D3D12_BLEND_ZERO,               /* α 現在のレンダーターゲットのアルファ値に対して実行するオプションを指定 */
			D3D12_BLEND_OP_ADD,             /* 結合 上二つをどのように結合するかを定義するオプション */
			D3D12_LOGIC_OP_NOOP,            /* レンダーターゲットに対して設定する論理演算を指定（今回は走査を行わないので *NOOP） */
			D3D12_COLOR_WRITE_ENABLE_ALL    /* レンダーターゲットの書き込みマスクを指定（今回はすべての値に対して書き込みを行う） */
		};

		// ブレンドステートの設定
		D3D12_BLEND_DESC BlendStateDesc = {};
		BlendStateDesc.AlphaToCoverageEnable  = FALSE;                         /* ピクセルシェーダーの出力アルファ値成分を取得し、アンチエイリアシング処理を有効にする（使用しない） */
		BlendStateDesc.IndependentBlendEnable = FALSE;                         /* FALSEの場合下のレンダーターゲットが0のみになる（１～７は無視される） */
		for (auto i = 0u; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {   /* レンダーターゲットが複数（ダブルバッファ用を除く）ある場合 */
			BlendStateDesc.RenderTarget[i]    = RTVBlendDesc;
		}



		// シェーダーの設定
		ComPtr<ID3DBlob> pVSBlob;  /* 頂点シェーダー */
		ComPtr<ID3DBlob> pPSBlob;  /* ピクセルシェーダー */


		// コンパイルされたシェーダーを読み込む
		std::wstring vsPath;
		std::wstring psPath;

		if (!SearchFilePath(L"VertexShader.cso", vsPath)) {

			return false;
		}

		if (!SearchFilePath(L"PixelShader.cso", psPath)) {

			return false;
		}


		// 頂点シェーダーとピクセルシェーダーを読み込む（コンパイル済みのシェーダーを読み込む。cso : compiled sheder object）
		HRESULT hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr)) {

			return false;
		}

		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr)) {

			return false;
		}


		// 深度ステンシルステートの設定
		D3D12_DEPTH_STENCIL_DESC DSSdesc = {};
		DSSdesc.DepthEnable    = TRUE;                              /* 深度テストを行うかどうか */
		DSSdesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;        /* 深度ステンシルバッファへの書き込みを有効にする */
		DSSdesc.DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;  /* 比較の際の関数（今回は「以下」を設定） */
		DSSdesc.StencilEnable  = FALSE;                             /* ステンシルテストを行うかどうか */



		// パイプラインステートの生成
		D3D12_GRAPHICS_PIPELINE_STATE_DESC GPdesc = {};
		GPdesc.InputLayout                     = { InputElement, _countof(InputElement) };                   /* 入力要素の設定。_count():配列の要素数を取得 */
		GPdesc.pRootSignature                  = m_pRootSignature.Get();                                     /* ルートシグネチャの設定 */
		GPdesc.VS                              = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };  /* 頂点シェーダーを指定 */
		GPdesc.PS                              = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };  /* ピクセルシェーダーを指定 */
		GPdesc.RasterizerState                 = RSdesc;                                                     /* ラスタライザーの設定を指定 */
		GPdesc.BlendState                      = BlendStateDesc;                                             /* ブレンドステートの設定を指定 */
		GPdesc.DepthStencilState               = DSSdesc;                                                    /* 深度ステンシルの設定 */
		GPdesc.SampleMask                      = UINT_MAX;                                                   /* ブレンド状態のサンプルマスク */
		GPdesc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;                     /* 三角形を入力として扱う */
		GPdesc.NumRenderTargets                = 1;                                                          /* RTVFormatsメンバー内のレンダーターゲット形式の数 */
		GPdesc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                            /* レンダーターゲット形式の */
		GPdesc.DSVFormat                       = DXGI_FORMAT_D32_FLOAT;                                      /* 深度ステンシルビューのフォーマットを指定（今回は使用しない） */
		GPdesc.SampleDesc.Count                = 1;                                                          /* マルチサンプリングの指定（アンチエイリアス処理だが０だと何もしないので１） */
		GPdesc.SampleDesc.Quality              = 0;                                                          /* マルチサンプリングの品質を指定（アンチエイリアス処理は使用しないので０） */


		// パイプラインステートの生成
		hr = m_pDevice->CreateGraphicsPipelineState(
			&GPdesc,
			IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr)) {

			std::cout << "パイプラインステートのエラー" << std::endl;
			return false;
		}
	}


	// ビューポートとシザー矩形の設定
	{
		m_Viewport.TopLeftX = 0;                             /* ビューポートのｘ座標の基準 */
		m_Viewport.TopLeftY = 0;                             /* ビューポートのｙ座標の基準 */
		m_Viewport.Width    = static_cast<float>(m_Width);   /* ビューポートの幅（intからfloatにキャスト）*/
		m_Viewport.Height   = static_cast<float>(m_Height);  /* ビューポートの高さ（intからfloatにキャスト）*/
		m_Viewport.MinDepth = 0.0f;                          /* ビューポートの最小深度 */
		m_Viewport.MaxDepth = 1.0f;                          /* ビューポートの最大深度 */

		m_Scissor.left   = 0;                                 /* 刈り取る領域の左 x座標 */
		m_Scissor.right  = m_Width;                           /* 刈り取る領域の右 x座標 */
		m_Scissor.top    = 0;                                 /* 刈り取る領域の上 y座標 */
		m_Scissor.bottom = m_Height;                          /* 刈り取る領域の下 y座標 */
	}
	

	// 正常終了
	return true;
}


void App::MainLoop() {

	// メッセージ情報を保存する変数
	MSG msg = {};

	/* 終了メッセージが送信されるまでループさせる */
	while (WM_QUIT != msg.message) {

		/* メッセージを受け取ったら処理する */
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {

			TranslateMessage(&msg);  // キーメッセージを文字メッセージに変換
			DispatchMessage(&msg);   // ウィンドウプロシージャに送信
		}
		else {

			Update();   // 更新処理
			Render();   // 描画処理
		}
	}

}


void App::Update() {

	//static float rotate = 0;
	//rotate += 0.025f;

	// １つ目のポリゴン
	//m_CBV[m_FrameIndex * 2 + 0].pBuffer->World = DirectX::XMMatrixRotationY(rotate + DirectX::XMConvertToRadians(45.0f));

	// 2つ目のポリゴン
	//m_CBV[m_FrameIndex * 2 + 1].pBuffer->World = DirectX::XMMatrixRotationY(rotate);
}


void App::Render() {

	// コマンドの記録を開始する
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(
		m_pCmdAllocator[m_FrameIndex].Get(),    /* コマンドアロケーターの指定 */
		nullptr);                               /* パイプラインステートの設定 */


	// リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;   /* リソースバリアのタイプを指定（今回は遷移）*/
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;         /* フラグの指定 */
	barrier.Transition.pResource   = m_pColorBuffer[m_FrameIndex].Get();       /* バリアを行うリソースの先頭アドレス */
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;             /* 前状態：表示 */
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;       /* 後状態：レンダーターゲット */
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  /* 全てのサブリソースを一度に遷移させる */

	// 設定したリソースバリアを割り当てる
	m_pCmdList->ResourceBarrier(1, &barrier);



	// レンダーターゲット（バックバッファ）の設定
	{
		m_pCmdList->OMSetRenderTargets(
			1,                            /* 設定するレンダーターゲットビュー用のディスクリプタハンドルの数を指定 */
			&m_HandleRTV[m_FrameIndex],   /* 設定するレンダーターゲットビュー用のディスクリプタハンドルの配列（ポインタ）を指定 */
			FALSE,                        /* ディスクリプタハンドルを共有させるか(TRUE) 独立させるか(FALSE) を指定（ほとんどの場合は独立させる） */
			&m_HandleDSV);                /* 設定する深度ステンシルビュー（奥行きを記憶するバッファのビュー）を設定する */

		// クリアカラーの設定
		static float frame = 0.0f;
		float r_color = cos(frame) * 0.5f + 0.5f;
		float g_color = sin(frame) * 0.5f + 0.5f;
		float clearColor[] = { r_color, g_color, 0.55f, 1.0f };
		frame += 0.03f;

		// レンダーターゲットビューをクリア
		m_pCmdList->ClearRenderTargetView(
			m_HandleRTV[m_FrameIndex],        /* レンダーターゲットビューをクリアするためのディスクリプタハンドルを指定 */
			clearColor,                       /* レンダーターゲットを指定の色でクリアするための色を指定 */
			0,                                /* 設定する矩形の数を指定（今回は使用しない）*/
			nullptr);                         /* レンダーターゲットをクリアするための矩形の配列を指定する（nullptrの場合全体がクリアされる） */
	}

	// 深度ステンシルビューをクリア
	{
		m_pCmdList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}


	// 描画処理
	{
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());                                  /* ルートシグネチャを送信 */
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV_SRV_UAV.GetAddressOf());                          /* CBV・SRV・UAVを送信 */
		m_pCmdList->SetGraphicsRootDescriptorTable(1, m_Texture.HandleGPU);          /* テクスチャ */
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].CBVDesc.BufferLocation);  /* 定数バッファの設定を送信 */

		m_pCmdList->SetPipelineState(m_pPSO.Get());                                                    /* パイプラインステートの設定 */

		m_pCmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   /* 入力が三角形要素であることを指定 */

		// １つ目のポリゴン
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);                                /* 頂点バッファを入力として指定 */
		m_pCmdList->IASetIndexBuffer(&m_IBV);                                        /* インデックスバッファビューを指定 */
		m_pCmdList->RSSetViewports(1, &m_Viewport);                                  /* ビューポートの設定 */
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);                                /* シザー矩形の設定 */
		m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);                             /* 描画 */

		// ２つ目のポリゴン
		//m_pCmdList->SetGraphicsRootDescriptorTable(1, m_Texture.HandleGPU);
		//m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex * 2 + 1].CBVDesc.BufferLocation);  /* 定数バッファの設定を送信 */
		//m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}



	// バリアをPresent状態（表示状態）に設定する
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource   = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;      /* 前状態：レンダーターゲット */
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;            /* 後状態：表示 */
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// 設定したバリアを割り当てる
	m_pCmdList->ResourceBarrier(1, &barrier);


	// コマンドリストの記録を終了
	m_pCmdList->Close();


	// コマンドの実行（コマンドキューで実行、コマンドリストを複数設定することも可能）
	ID3D12CommandList* CmdLists[] = { m_pCmdList.Get() };
	m_pCmdQueue->ExecuteCommandLists(
		1,                             /* コマンドリストの数を指定（複数のコマンドリストを登録可能） */
		CmdLists);                     /* コマンドリストをまとめた配列 */


	// 画面に表示
	Present(1);
}


void App::Present(uint32_t interval) {

	// 画面に表示
	m_pSwapChain->Present(interval, 0);


	// シグナル処理
	const uint64_t currentCount = m_FenceCounter[m_FrameIndex];
	m_pCmdQueue->Signal(m_pFence.Get(), currentCount);


	// バックバッファ番号の更新
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();


	// 次のフレームの描画準備がまだの場合は待機（フェンスの値が更新されていなければ待機）
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex]) {

		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}


	// 次のフレームのフェンスカウンターを増やす
	m_FenceCounter[m_FrameIndex] = currentCount + 1;

}


void App::WaitGPU() {

	// ポインタが NULLの場合エラー
	assert(m_pCmdQueue  != nullptr);
	assert(m_pFence     != nullptr);
	assert(m_FenceEvent != nullptr);


	// シグナルの処理
	m_pCmdQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);


	// 完了時にイベント設定する
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);


	//　GPUの処理完了待機
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);


	// カウンターを増やす
	m_FenceCounter[m_FrameIndex]++;
}


void App::TermApp() {

	// 描画インターフェイスの破棄
	OnTerm();

	// Direct3Dの破棄
	TermDirect3D();

	// ウィンドウの破棄
	TermWindow();

}


void App::OnTerm() {

	// 定数バッファを破棄する
	for (auto i = 0u; i < FrameCount; i++) {

		if (m_pCB[i] != nullptr){

			m_pCB[i]->Unmap(0, nullptr);             // アンマップ
			memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
		}
		m_pCB[i].Reset();
	}

	m_pHeapCBV_SRV_UAV.Reset();  // CBV・SRV・UAVのディスクリプタヒープを破棄


	m_pPSO.Reset();              // パイプラインステートの破棄
}


void App::TermDirect3D() {

	// GPUの処理が完了するまで待機
	WaitGPU();


	/* 生成した順番と逆順に解放していく */
	
	// イベントの破棄
	if (m_FenceEvent != nullptr) {

		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}


	// フェンスの破棄
	m_pFence.Reset();


	// レンダーターゲットビューの破棄
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; i++) {

		m_pColorBuffer[i].Reset();
	}


	// コマンドリストの破棄
	m_pCmdList.Reset();


	// コマンドアロケーターの破棄
	for (auto i = 0u; i < FrameCount; i++) {

		m_pCmdAllocator[i].Reset();
	}


	// スワップチェインの破棄
	m_pSwapChain.Reset();


	// コマンドキューの破棄
	m_pCmdQueue.Reset();


	// デバイスの破棄
	m_pDevice.Reset();
	
}



void App::TermWindow() {

	// ウィンドウ登録の解除
	if (m_hInst != nullptr) {
		UnregisterClass(m_windowTitle, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
		
		/* ウィンドウが破棄される場合に送信される */
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			break;
	}

	/* 他のメッセージを処理してくれる関数を返す */
	return DefWindowProc(hwnd, msg, wp, lp);
}


template<typename T>
void SafeRelease(T* ptr) {

	if (ptr != nullptr) {

		ptr->Release();
		ptr = nullptr;
	}
}