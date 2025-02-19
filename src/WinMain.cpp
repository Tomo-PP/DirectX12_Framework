
/****************************************************************
 * Include
 ****************************************************************/
#include "WinMain.h"
#include <iostream>
#include <string>
#include <SimpleMath.h>


#if defined (DEBUG) || defined (_DEBUG)
	ComPtr<ID3D12DebugDevice> debugDevice;
#endif


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
	m_pFence      (nullptr),

	//m_pVB         (nullptr),
	m_pPSO        (nullptr),
	m_FrameIndex  (0),
	//m_CBV         (),
	m_Scissor     (),
	m_Viewport    ()
{
	for (auto i = 0u; i < FrameCount; i++) {

		m_pCmdAllocator[i] = nullptr;
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
	m_pDevice->SetName(L"GPUDevice");

#if defined (DEBUG) || defined (_DEBUG)
	m_pDevice.As(&debugDevice);
	
#endif


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
		m_pCmdQueue->SetName(L"CommandQueue");
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

		m_pCmdAllocator[0]->SetName(L"CommandAllocator_1");
		m_pCmdAllocator[1]->SetName(L"CommandAllocator_2");
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

		m_pCmdList->SetName(L"CommandList");
	}


	// レンダーターゲットビューの生成
	if (!m_DespManager.CreateRTV(m_pDevice.Get(), m_pSwapChain.Get())) {

		return false;
	}


	// 深度ステンシルビューの生成
	if (!m_DespManager.CreateDSV(m_pDevice.Get(), m_Width, m_Height)) {

		return false;
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

		m_pFence->SetName(L"Fence");
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

	// Imguiの初期化
	if (!Init_Imgui()) {

		std::cout << "Error : Can't Initialize Imgui." << std::endl;
		return false;
	}


	// 正常終了
	return true;
}



bool App::OnInit() {

	// Imguiの初期化
	{
		if (ImGui::CreateContext() == nullptr) {

			std::cout << "Imguiの初期化に失敗" << std::endl;
			return false;
		}

		// Windowの初期化
		bool blnResult = ImGui_ImplWin32_Init(m_hWnd);
		if (!blnResult) {

			std::cout << "ImguiのWindow初期化に失敗" << std::endl;
			return false;
		}

		blnResult = ImGui_ImplDX12_Init(
			m_pDevice.Get(),                                         // デバイス
			3,                                                       // フレーム
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,                         // RTVフォーマット
			m_pHeapForImgui.Get(),                                   // Imguiヒープ
			m_pHeapForImgui->GetCPUDescriptorHandleForHeapStart(),   // CPUハンドル
			m_pHeapForImgui->GetGPUDescriptorHandleForHeapStart());  // GPUハンドル

		if (!blnResult) {

			std::cout << "Imgui初期化に失敗" << std::endl;
			return false;
		}

	}


	// ヒープサイズを設定
	size_t size = 5;
	m_DespManager.Init_CBV_SRV_UAV(m_pDevice.Get(), size);

	//// メッシュをロード
	//{

	//	// パスの検索
	//	std::wstring path;
	//	if (!SearchFilePath(L"Floor/Untitled.obj", path)) {

	//		std::cout << "OBJファイルが見つかりません" << std::endl;
	//		return false;
	//	}

	//	// メッシュのロード
	//	if (!LoadMesh(path.c_str(), m_meshes, m_materials)) {

	//		std::cout << "OBJファイルがロードできません" << std::endl;
	//		return false;
	//	}
	//}



	//// 頂点バッファの生成
	//{

	//	// ヒーププロパティ（データをどう送るのかを記述）
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（今回は指定なし）*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */


	//	// バッファサイズを決定
	//	auto VertexSize = sizeof(MeshVertex) * m_meshes[0].Vertices.size();  /* MeshVertex（頂点情報）× 頂点情報の数 */
	//	auto vertices = m_meshes[0].Vertices.data();                         /* マッピング用の頂点データを確保する（可変配列の先頭ポインタを取得）*/

	//	// リソースの設定
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
	//	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	//	resourceDesc.Width              = UINT64(VertexSize);               /* 頂点情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
	//	resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
	//	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/

	//	// 頂点バッファ（GPUリソース）を生成
	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,                                   /* ヒープの設定 */
	//		D3D12_HEAP_FLAG_NONE,                        /* ヒープのオプション */
	//		&resourceDesc,                               /* リソースの設定 */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,           /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
	//		nullptr,                                     /* RTVとDSV用の設定 */
	//		IID_PPV_ARGS(m_pVB.GetAddressOf()));         /* アドレスを格納 */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// 頂点バッファのマッピングを行う
	//	void* ptr = nullptr;
	//	hr = m_pVB->Map(0, nullptr, &ptr);  //　上で設定したサイズ分のGPUリソースへのポインタを取得
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// 頂点データをマッピング先に設定（GPUのメモリをコピー）
	//	memcpy(ptr, vertices, VertexSize);

	//	// マッピングの解除
	//	m_pVB->Unmap(0, nullptr);


	//	// 頂点バッファビューの設定（頂点バッファの描画コマンド用・GPUのアドレスやサイズなどを記憶しておく）
	//	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();                          /* 先ほどマップしたGPUの仮想アドレスを記憶 */
	//	m_VBV.SizeInBytes    = static_cast<UINT>(VertexSize);                          /* 頂点データ全体のサイズを記憶 */
	//	m_VBV.StrideInBytes  = static_cast<UINT>(sizeof(MeshVertex));                  /* １頂点辺りのサイズを記憶 */
	//}




	//// インデックスバッファの生成
	//{
	//	

	//	// ヒーププロパティ
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /*  */
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /*  */
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /*  */
	//	heapProp.CreationNodeMask     = 1;                                /*  */
	//	heapProp.VisibleNodeMask      = 1;                                /*  */


	//	// インデックスバッファのサイズを決定
	//	auto IndexSize = sizeof(uint32_t) * m_meshes[0].Indices.size();  /* インデックスはuint32_t型 × インデックスの数 */
	//	auto indices   = m_meshes[0].Indices.data();                     /* マッピング用インデックスデータの先頭ポインタを取得 */

	//	// リソースの設定
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定）*/
	//	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	//	resourceDesc.Width              = IndexSize;                        /* インデックス情報が入るサイズのバッファサイズ（テクスチャの場合は横幅を指定）*/
	//	resourceDesc.Height             = 1;                                /* バッファの場合は１（テクスチャの場合は縦幅を指定）*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* リソースの奥行（バッファ・テクスチャは１、三次元テクスチャは奥行）*/
	//	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（今回はバッファなので *UNKNOWN。テクスチャの場合はピクセルフォーマットを指定）*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定 */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* バッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する）*/
	//	

	//	// リソースの生成
	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,
	//		&resourceDesc,
	//		D3D12_RESOURCE_STATE_GENERIC_READ,
	//		nullptr,
	//		IID_PPV_ARGS(m_pIB.GetAddressOf()));
	//	if (FAILED(hr)) {
	//		std::cout << "インデックスバッファのエラー" << std::endl;
	//		return false;
	//	}


	//	// マッピングを行う
	//	void* ptr = nullptr;
	//	hr = m_pIB->Map(0, nullptr, &ptr);
	//	if (FAILED(hr)) {

	//		return false;
	//	}

	//	// インデックスデータをGPUメモリにコピーする
	//	memcpy(ptr, indices, IndexSize);


	//	// マッピングの解除
	//	m_pIB->Unmap(0, nullptr);


	//	// インデックスバッファビューの設定
	//	m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();  /* インデックスバッファのGPUメモリ */
	//	m_IBV.Format         = DXGI_FORMAT_R32_UINT;           /* フォーマット（ポリゴン数が多い場合 *R32_UINT ポリゴン数が少ない場合 *R16_UINT） */
	//	m_IBV.SizeInBytes    = static_cast<UINT>(IndexSize);   /* インデックスデータのデータサイズ（バイト）*/
	//}



	//// CBV / SRV / UAV用のディスクリプタヒープの生成
	//{

	//	// ディスクリプタヒープの設定
	//	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//	heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     /* 定数バッファを含んだフラグを指定 */
	//	heapDesc.NumDescriptors = 10;                                         /* ディスクリプタの数 */
	//	heapDesc.NodeMask       = 0;                                          /* GPUは１つなので０を指定 */
	//	heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  /* シェーダー側から参照できるようにする */

	//	// ディスクリプタヒープの生成
	//	HRESULT hr = m_pDevice->CreateDescriptorHeap(
	//		&heapDesc,
	//		IID_PPV_ARGS(m_pHeapCBV_SRV_UAV.GetAddressOf()));
	//	if (FAILED(hr)) {

	//		return false;
	//	}
	//}
	//


	//// 定数バッファの生成（座標行列などをシェーダーに渡すバッファ）
	//{
	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                 // アライメントサイズ（256 Byte）
	//	uint64_t alignmentSize = (sizeof(Transform) + (align - 1)) & ~(align - 1);   // アライメントサイズの計算

	//	// ヒーププロパティ（データをどう送るのかを記述）
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（CPUの書き込み方法について・今回は指定なし）*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */

	//	// リソースの設定
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定） */
	//	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	//	resourceDesc.Width              = alignmentSize;                    /* データサイズ：256 Byte（256 Byteを超える場合 512 Byte）*/
	//	resourceDesc.Height             = 1;                                /* データの縦のサイズ（バッファなので１）*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* データの奥行（バッファなので１）*/
	//	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定（今回は使わないので０） */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* 始まりから終わりまで連続したバッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する） */


	//	// ディスクリプタヒープ内のデータについて、次のデータに移動するためのインクリメントサイズを取得
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	//	// バックバッファの数分の定数バッファを生成（GPUリソース）
	//	for (auto i = 0u; i < FrameCount; i++) {

	//		HRESULT hr = m_pDevice->CreateCommittedResource(
	//			&heapProp,                               /* ヒープの設定 */
	//			D3D12_HEAP_FLAG_NONE,                    /* ヒープのオプション */
	//			&resourceDesc,                           /* リソースの設定 */
	//			D3D12_RESOURCE_STATE_GENERIC_READ,       /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
	//			nullptr,                                 /* RTVとDSV用の設定 */
	//			IID_PPV_ARGS(m_pCB[i].GetAddressOf()));  /* アドレスを格納 */
	//		if (FAILED(hr)) {

	//			return false;
	//		}

	//		// 定数バッファのアドレス
	//		auto addressGPU = m_pCB[i]->GetGPUVirtualAddress();                          // GPUの仮想アドレスを取得
	//		auto handleCPU  = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（CPU）
	//		auto handleGPU  = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  // ディスクリプタヒープの先頭ハンドルを取得（GPU）

	//		// 定数バッファの先頭ポインタを計算（定数バッファのサイズ分をインクリメント）
	//		handleCPU.ptr += incrementSize * i;
	//		handleGPU.ptr += incrementSize * i;

	//		// 定数バッファビューの設定（定数バッファの情報を保存）
	//		m_CBV[i].HandleCPU              = handleCPU;          // 定数バッファの先頭ハンドル（CPU）
	//		m_CBV[i].HandleGPU              = handleGPU;          // 定数バッファの先頭ハンドル（GPU）
	//		m_CBV[i].CBVDesc.BufferLocation = addressGPU;         // バッファの保存位置を指定
	//		m_CBV[i].CBVDesc.SizeInBytes    = alignmentSize;      // 定数バッファのサイズ

	//		// 定数バッファビューの生成
	//		m_pDevice->CreateConstantBufferView(&m_CBV[i].CBVDesc, handleCPU);


	//		//定数バッファ（Transform）のマッピングを行う
	//		hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));  // 上で設定したサイズ分のGPUリソースへのポインタを取得
	//		if (FAILED(hr)) {
	//			std::cout << "定数バッファのマップエラー" << std::endl;
	//			return false;
	//		}

	//		// マッピングした行列を設定する
	//		auto eyePos = DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);     /* カメラ座標 */
	//		auto targetPos = DirectX::XMVectorZero();                       /* 注視点座標（原点）*/
	//		auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);     /* カメラの高さ */

	//		auto fovY = DirectX::XMConvertToRadians(37.5f);                           /* カメラの Y軸に対する画角 */
	//		auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height); /* 高さに対する幅の割合 */

	//		// 変換行列の設定
	//		m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity() * DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);                                          /* ワールド行列・Identityは単位行列 */
	//		m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);                  /* カメラ行列 */
	//		m_CBV[i].pBuffer->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);  /* 射影行列 */
	//	}

	//	m_CBV[0].pBuffer->World *= DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//	m_CBV[1].pBuffer->World *= DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//}


	//// ライトの作成
	//{
	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                    // アライメントサイズ（256 Byte）
	//	uint64_t alignmentSize = (sizeof(LightBuffer) + (align - 1)) & ~(align - 1);    // アライメントサイズの計算

	//	// ヒーププロパティ（データをどう送るのかを記述）
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（CPUの書き込み方法について・今回は指定なし）*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */

	//	// リソースの設定
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定） */
	//	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	//	resourceDesc.Width              = alignmentSize;                    /* データサイズ：256 Byte（256 Byteを超える場合 512 Byte）*/
	//	resourceDesc.Height             = 1;                                /* データの縦のサイズ（バッファなので１）*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* データの奥行（バッファなので１）*/
	//	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定（今回は使わないので０） */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* 始まりから終わりまで連続したバッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する） */


	//	// ディスクリプタヒープ内のデータについて、次のデータに移動するためのインクリメントサイズを取得
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,                    /* ヒープのオプション */
	//		&resourceDesc,                           /* リソースの設定 */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,       /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
	//		nullptr,                                 /* RTVとDSV用の設定 */
	//		IID_PPV_ARGS(m_pCB[2].GetAddressOf()));  /* アドレスを格納 */
	//	if (FAILED(hr)) {

	//		std::cout << "Error : Can't create Material Resource." << std::endl;
	//		return false;
	//	}

	//	auto virtualAddress = m_pCB[2]->GetGPUVirtualAddress();                          /* GPUのアドレス */
	//	auto handleCPU      = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  /* CPUヒープの先頭 */
	//	auto handleGPU      = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  /* GPUヒープの先頭 */

	//	// 行列変換用のCBV分を飛ばす
	//	handleCPU.ptr += incrementSize * 2;
	//	handleGPU.ptr += incrementSize * 2;

	//	// 定数バッファビュー
	//	m_LightCBV.HandleCPU              = handleCPU;         /* CPU仮想アドレス */
	//	m_LightCBV.HandleGPU              = handleGPU;         /* GPUヒープの先頭 */
	//	m_LightCBV.CBVDesc.BufferLocation = virtualAddress;    /* GPU仮想アドレス */
	//	m_LightCBV.CBVDesc.SizeInBytes    = static_cast<UINT>(alignmentSize);     /* バッファのサイズ（アライメントサイズ）*/

	//	// 定数バッファビューの生成
	//	m_pDevice->CreateConstantBufferView(&m_LightCBV.CBVDesc, handleCPU);


	//	//定数バッファ（Material）のマッピングを行う
	//	hr = m_pCB[2]->Map(0, nullptr, reinterpret_cast<void**>(&m_LightCBV.pBuffer));  // 上で設定したサイズ分のGPUリソースへのポインタを取得
	//	if (FAILED(hr)) {
	//		std::cout << "ライト用定数バッファのマップエラー" << std::endl;
	//		return false;
	//	}

	//	m_LightCBV.pBuffer->LightPosition = Vector4(10.0f, 0.0f, 0.0f, 0.0f);  /* ライト座標 */
	//	m_LightCBV.pBuffer->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);    /* ライト色 */
	//}


	//// マテリアルの生成（CBV）
	//{

	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                 // アライメントサイズ（256 Byte）
	//	uint64_t alignmentSize = (sizeof(Material) + (align - 1)) & ~(align - 1);    // アライメントサイズの計算

	//	// ヒーププロパティ（データをどう送るのかを記述）
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* ヒープのタイプを指定（今回はGPUに送る用のヒープ）*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPUページプロパティ（CPUの書き込み方法について・今回は指定なし）*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* メモリプールの扱いについて（今回は指定なし）*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPUの数 */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPUの識別する数 */

	//	// リソースの設定
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* 扱うリソースの次元を設定（頂点バッファなので *BUFFERを指定） */
	//	resourceDesc.Alignment          = 0;                                /* メモリの区切る量 *BUFFERの場合は 64 KBまたは 0を指定 */
	//	resourceDesc.Width              = alignmentSize;                    /* データサイズ：256 Byte（256 Byteを超える場合 512 Byte）*/
	//	resourceDesc.Height             = 1;                                /* データの縦のサイズ（バッファなので１）*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* データの奥行（バッファなので１）*/
	//	resourceDesc.MipLevels          = 1;                                /* ミップマップのレベルの設定（バッファの場合は１） */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* データのフォーマットを指定（テクスチャの場合はピクセルフォーマットを指定）*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* アンチエイリアシングの設定（０だとデータがないことになってしまう）*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* アンチエイリアシングの設定（今回は使わないので０） */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* 始まりから終わりまで連続したバッファなので *MAJOR（テクスチャの場合は *UNKNOWN）*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* 今回は設定なし（RTV・DSV・UAV・SRVの場合は設定する） */


	//	// ディスクリプタヒープ内のデータについて、次のデータに移動するためのインクリメントサイズを取得
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,                    /* ヒープのオプション */
	//		&resourceDesc,                           /* リソースの設定 */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,       /* リソースの初期状態を指定（ヒープ設定で *UPLOADにした場合 *GENERIC_READを指定）*/
	//		nullptr,                                 /* RTVとDSV用の設定 */
	//		IID_PPV_ARGS(m_pCB[3].GetAddressOf()));  /* アドレスを格納 */
	//	if (FAILED(hr)) {

	//		std::cout << "Error : Can't create Material Resource." << std::endl;
	//		return false;
	//	}

	//	auto virtualAddress = m_pCB[3]->GetGPUVirtualAddress();                          /* GPUのアドレス */
	//	auto handleCPU      = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  /* CPUヒープの先頭 */
	//	auto handleGPU      = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  /* GPUヒープの先頭 */

	//	// 行列変換用のCBV分を飛ばす
	//	handleCPU.ptr += incrementSize * 3;
	//	handleGPU.ptr += incrementSize * 3;

	//	// 定数バッファビュー
	//	m_Material.HandleCPU              = handleCPU;         /* CPU仮想アドレス */
	//	m_Material.HandleGPU              = handleGPU;         /* GPUヒープの先頭 */
	//	m_Material.CBVDesc.BufferLocation = virtualAddress;    /* GPU仮想アドレス */
	//	m_Material.CBVDesc.SizeInBytes    = static_cast<UINT>(alignmentSize);     /* バッファのサイズ（アライメントサイズ）*/

	//	// 定数バッファビューの生成
	//	m_pDevice->CreateConstantBufferView(&m_Material.CBVDesc, handleCPU);


	//	//定数バッファ（Material）のマッピングを行う
	//	hr = m_pCB[3]->Map(0, nullptr, reinterpret_cast<void**>(&m_Material.pBuffer));  // 上で設定したサイズ分のGPUリソースへのポインタを取得
	//	if (FAILED(hr)) {
	//		std::cout << "マテリアル用定数バッファのマップエラー" << std::endl;
	//		return false;
	//	}

	//	// メッシュから定数バッファを取得
	//	Material material = m_materials[0];

	//	// マテリアルの設定
	//	m_Material.pBuffer->Diffuse   = material.Diffuse;
	//	m_Material.pBuffer->alpha     = material.alpha;
	//	m_Material.pBuffer->Specular  = material.Specular;
	//	m_Material.pBuffer->Shininess = material.Shininess;

	//	std::cout << "Diffuse : " << m_Material.pBuffer->Diffuse.x << ", " << m_Material.pBuffer->Diffuse .y << ", " << m_Material.pBuffer->Diffuse.z << std::endl;
	//	std::cout << "alpha : " << m_Material.pBuffer->alpha << std::endl;
	//	std::cout << "Specular : " << m_Material.pBuffer->Specular.x << ", " << m_Material.pBuffer->Specular.y << ", " << m_Material.pBuffer->Specular.z << std::endl;
	//	std::cout << "Shininess : " << m_Material.pBuffer->Shininess << std::endl;
	//}



	//// テクスチャの生成（SRV）
	//{
	//	// ファイルパスの検索
	//	std::wstring texturePath;
	//	if (!SearchFilePath(L"Floor/BrickRound.dds", texturePath)) {

	//		return false;
	//	}


	//	// 画像データの読み取り処理
	//	DirectX::ResourceUploadBatch batch(m_pDevice.Get());
	//	batch.Begin();                                        // 内部処理でコマンドアロケーターとコマンドリストが生成される

	//	// リソースの生成（この関数で一気にリソースを生成できる。リソース設定からしなくてもよい）
	//	HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//		m_pDevice.Get(),                     /* デバイスを渡す */
	//		batch,                               /*  */
	//		texturePath.c_str(),                 /* テクスチャまでのパス（ワイド文字をマルチバイトに変換して渡す必要あり）*/
	//		m_Texture.pResource.GetAddressOf(),  /* テクスチャのアドレスを取得 */
	//		true);                               /*  */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// コマンドの実行
	//	auto future = batch.End(m_pCmdQueue.Get());

	//	// コマンド完了まで待機
	//	future.wait();

	//	// インクリメントサイズを取得（シェーダー用（SRV）のメモリのインクリメントサイズ）
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	// CPUディスクリプタハンドルとGPUディスクリプタハンドルをディスクリプタヒープから取得
	//	auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
	//	auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

	//	// テクスチャにディスクリプタハンドルを割り当てる（CBV×4を飛ばす必要あり）
	//	handleCPU.ptr += incrementSize * 4;
	//	handleGPU.ptr += incrementSize * 4;

	//	m_Texture.HandleCPU = handleCPU;
	//	m_Texture.HandleGPU = handleGPU;

	//	// テクスチャの構成を取得（上のライブラリで生成された構成を読み込む）
	//	auto textureDesc = m_Texture.pResource->GetDesc();

	//	// シェーダリソースビューの設定
	//	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	//	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	//	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	//	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	//	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	//	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	//	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	//	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	//	// シェーダーリソースビューの生成
	//	m_pDevice->CreateShaderResourceView(
	//		m_Texture.pResource.Get(),         /*  */
	//		&SRVDesc,                          /*  */
	//		handleCPU);                        /*  */
	//}


	//// 法線マップの生成（SRV）
	//{
	//	// ファイルパスの検索
	//	std::wstring texturePath;
	//	if (!SearchFilePath(L"Floor/BrickRoundBUMP.dds", texturePath)) {

	//		std::cout << "Error : Can't find Texture." << std::endl;
	//		return false;
	//	}


	//	// 画像データの読み取り処理
	//	DirectX::ResourceUploadBatch batch(m_pDevice.Get());
	//	batch.Begin();                                        // 内部処理でコマンドアロケーターとコマンドリストが生成される

	//	// リソースの生成（この関数で一気にリソースを生成できる。リソース設定からしなくてもよい）
	//	HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//		m_pDevice.Get(),                        /* デバイスを渡す */
	//		batch,                                  /*  */
	//		texturePath.c_str(),                    /* テクスチャまでのパス（ワイド文字をマルチバイトに変換して渡す必要あり）*/
	//		NormalMapSRV.pResource.GetAddressOf(),  /* テクスチャのアドレスを取得 */
	//		true);                                  /*  */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// コマンドの実行
	//	auto future = batch.End(m_pCmdQueue.Get());

	//	// コマンド完了まで待機
	//	future.wait();

	//	// インクリメントサイズを取得（シェーダー用（SRV）のメモリのインクリメントサイズ）
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	// CPUディスクリプタハンドルとGPUディスクリプタハンドルをディスクリプタヒープから取得
	//	auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
	//	auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

	//	// テクスチャにディスクリプタハンドルを割り当てる（CBV×4＋SRV×1を飛ばす必要あり）
	//	handleCPU.ptr += incrementSize * 5;
	//	handleGPU.ptr += incrementSize * 5;

	//	NormalMapSRV.HandleCPU = handleCPU;
	//	NormalMapSRV.HandleGPU = handleGPU;

	//	// テクスチャの構成を取得（上のライブラリで生成された構成を読み込む）
	//	auto textureDesc = NormalMapSRV.pResource->GetDesc();

	//	// シェーダリソースビューの設定
	//	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	//	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	//	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	//	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	//	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	//	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	//	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	//	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	//	// シェーダーリソースビューの生成
	//	m_pDevice->CreateShaderResourceView(
	//		NormalMapSRV.pResource.Get(),        /*  */
	//		&SRVDesc,                            /*  */
	//		handleCPU);                          /*  */
	//}


	// ここから上の処理を一つのオブジェクトクラスに処理としてまとめたい==============================================

	if (!model.Init(m_pDevice.Get(), m_pCmdQueue.Get(), &m_DespManager, L"Floor/untitled.obj")){

		return false;
	}



	// ルートシグネチャ生成（シェーダー内で使用するリソースの扱い方を決める）
	{

		// ルートシグネチャのレイアウトオプションの設定（論理和で指定・使用しないものを記述）
		auto flagLayout = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;     /* ハルシェーダーのルートシグネチャへのアクセスを拒否 */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;   /* ドメインシェーダーのルートシグネチャへのアクセスを拒否 */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; /* ジオメトリーシェーダーのルートシグネチャへのアクセスを拒否 */


		// ディスクリプターテーブル（SRV）の範囲を設定

		// テクスチャのSRV
		D3D12_DESCRIPTOR_RANGE range[2] = {};
		range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  /* ディスクリプタの種類を設定 */
		range[0].NumDescriptors                    = 1;                                /* ディスクリプターの数 */
		range[0].BaseShaderRegister                = 0;                                /* 始めるレジスタ番号 */
		range[0].RegisterSpace                     = 0;                                /*  */
		range[0].OffsetInDescriptorsFromTableStart = 0;                                /* オフセット */

		// 法線マップのSRV
		range[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  /* ディスクリプタの種類 */
		range[1].NumDescriptors                    = 1;                                /* ディスクリプタの数 */
		range[1].BaseShaderRegister                = 1;                                /* 始める番号 */
		range[1].RegisterSpace                     = 0;                                /*  */
		range[1].OffsetInDescriptorsFromTableStart = 0;                                /* オフセット */


		// ルートパラメーターの設定（CBV・SRVをシェーダーに送る | CBV | CBV | CBV | SRV | SRV | ）

		// 変換行列（CBV）
		D3D12_ROOT_PARAMETER rootParam[5] = {};
		rootParam[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;  /* ルートパラメーターのタイプは定数バッファ（CBV）*/
		rootParam[0].Descriptor.ShaderRegister = 0;                              /* レジスタの開始番号 */
		rootParam[0].Descriptor.RegisterSpace  = 0;                              /* 今回は使わない */
		rootParam[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX; /* 頂点シェーダーから参照できるようにする */

		// ライト行列（CBV）
		rootParam[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[1].Descriptor.ShaderRegister = 1;
		rootParam[1].Descriptor.RegisterSpace  = 0;
		rootParam[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// マテリアル行列（CBV）
		rootParam[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[2].Descriptor.ShaderRegister = 2;
		rootParam[2].Descriptor.RegisterSpace  = 0;
		rootParam[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// テクスチャ用（SRV）
		rootParam[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  /* ルートパラメーターのタイプ（ディスクリプターテーブル）*/
		rootParam[3].DescriptorTable.NumDescriptorRanges = 1;                                           /* レンジの数 */
		rootParam[3].DescriptorTable.pDescriptorRanges   = &range[0];                                   /* 設定したディスクリプタレンジを指定 */
		rootParam[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               /* ピクセルシェーダーで使用できるようにする */

		// 法線マップ用（SRV）
		rootParam[4].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  
		rootParam[4].DescriptorTable.NumDescriptorRanges = 1;                                           
		rootParam[4].DescriptorTable.pDescriptorRanges   = &range[1];                                   
		rootParam[4].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               


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
		rootSigDesc.NumParameters     = _countof(rootParam);  /* ルートパラメーターの数 */
		rootSigDesc.NumStaticSamplers = 1;                    /* スタティックサンプラーの数 */
		rootSigDesc.pParameters       = rootParam;            /* 定義したルートパラメーターを指定（配列の先頭アドレス）*/
		rootSigDesc.pStaticSamplers   = &SamplerDesc;         /* 先ほど設定したスタティックサンプラーを指定 */
		rootSigDesc.Flags             = flagLayout;           /* 指定したものがルートシグネチャをアクセスすることを拒否するフラグ */

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
		GPdesc.InputLayout                     = MeshVertex::InputLayout;                                    /* 入力要素の設定 */
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

	model.Update();

}


void App::Render() {

	// コマンドの記録を開始する
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(
		m_pCmdAllocator[m_FrameIndex].Get(),    /* コマンドアロケーターの指定 */
		nullptr);                               /* パイプラインステートの設定 */


	// リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;       /* リソースバリアのタイプを指定（今回は遷移）*/
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;             /* フラグの指定 */
	barrier.Transition.pResource   = m_DespManager.GetResource_RTB(m_FrameIndex);  /* バリアを行うリソースの先頭アドレス */
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;                 /* 前状態：表示 */
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;           /* 後状態：レンダーターゲット */
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;      /* 全てのサブリソースを一度に遷移させる */

	// 設定したリソースバリアを割り当てる
	m_pCmdList->ResourceBarrier(1, &barrier);



	// RTV・DSVの設定
	{
		auto handleRTV = m_DespManager.GetCPUHandle_RTV(m_FrameIndex);
		auto handleDSV = m_DespManager.GetCPUHandle_DSV();
		m_pCmdList->OMSetRenderTargets(
			1,                            /* 設定するレンダーターゲットビュー用のディスクリプタハンドルの数を指定 */
			&handleRTV,                   /* 設定するレンダーターゲットビュー用のディスクリプタハンドルの配列（ポインタ）を指定 */
			FALSE,                        /* ディスクリプタハンドルを共有させるか(TRUE) 独立させるか(FALSE) を指定（ほとんどの場合は独立させる） */
			&handleDSV);                  /* 設定する深度ステンシルビュー（奥行きを記憶するバッファのビュー）を設定する */

		// クリアカラーの設定
		float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };


		// レンダーターゲットビューをクリア
		m_pCmdList->ClearRenderTargetView(
			handleRTV,                       /* レンダーターゲットビューをクリアするためのディスクリプタハンドルを指定 */
			clearColor,                      /* レンダーターゲットを指定の色でクリアするための色を指定 */
			0,                               /* 設定する矩形の数を指定（今回は使用しない）*/
			nullptr);                        /* レンダーターゲットをクリアするための矩形の配列を指定する（nullptrの場合全体がクリアされる） */
	

		// 深度ステンシルビューをクリア
		m_pCmdList->ClearDepthStencilView(m_DespManager.GetCPUHandle_DSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}


	// 描画処理
	{
		auto heap = m_DespManager.GetHeapCBV_SRV_UAV();
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());     /* ルートシグネチャを送信 */
		m_pCmdList->SetDescriptorHeaps(1, &heap);                         /* CBV・SRV・UAVを送信 */
		m_pCmdList->SetPipelineState(m_pPSO.Get());                       /* パイプラインステートの設定 */

		m_pCmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   /* 入力が三角形要素であることを指定 */
		m_pCmdList->RSSetViewports(1, &m_Viewport);                                  /* ビューポートの設定 */
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);                                /* シザー矩形の設定 */

		// メッシュの描画
		model.Render(m_pCmdList.Get(), m_FrameIndex);
	}

	// Imguiの描画
	ImguiRender();


	// バリアをPresent状態（表示状態）に設定する
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource   = m_DespManager.GetResource_RTB(m_FrameIndex);
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


	m_pPSO.Reset();              // パイプラインステートの破棄

	//// テクスチャの破棄
	//m_Texture.pResource.Reset();
	//m_Texture.HandleCPU.ptr = 0;
	//m_Texture.HandleGPU.ptr = 0;

	//// 法線マップの破棄
	//NormalMapSRV.pResource.Reset();
	//NormalMapSRV.HandleCPU.ptr = 0;
	//NormalMapSRV.HandleGPU.ptr = 0;


	//// 定数バッファを破棄する
	//for (auto i = 0u; i < _countof(m_pCB); i++) {

	//	if (m_pCB[i] != nullptr){

	//		m_pCB[i]->Unmap(0, nullptr);             // アンマップ
	//		memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
	//	}
	//	m_pCB[i].Reset();
	//}

	//// メッシュの破棄
	//for (size_t i = 0; i < m_meshes.size(); i++) {

	//	m_meshes[i].Vertices.clear();
	//	m_meshes[i].Indices.clear();
	//}
	//m_meshes.clear();

	//// マテリアルの削除
	//m_materials.clear();


	//// バッファの破棄
	//m_pIB.Reset();
	//m_pVB.Reset();


	//m_pHeapCBV_SRV_UAV.Reset();  // CBV・SRV・UAVのディスクリプタヒープを破棄


	//// バッファビューの破棄
	//m_VBV.BufferLocation = 0;
	//m_VBV.SizeInBytes    = 0;
	//m_VBV.StrideInBytes  = 0;

	//m_IBV.BufferLocation = 0;
	//m_IBV.SizeInBytes    = 0;
	//m_IBV.Format         = DXGI_FORMAT_UNKNOWN;

	// モデルの破棄
	model.Term();


	// ルートシグネチャの破棄
	m_pRootSignature.Reset();

}


void App::TermDirect3D() {

	// GPUの処理が完了するまで待機
	WaitGPU();


	/* 生成した順番と逆順に解放していく */


	// Imguiの破棄
	m_pHeapForImgui.Reset();
	Term_Imgui();

	
	// イベントの破棄
	if (m_FenceEvent != nullptr) {

		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}


	// フェンスの破棄
	m_pFence.Reset();


	// RTV・DSVの破棄
	m_DespManager.Term();


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

#if defined (DEBUG) || defined (_DEBUG)

	/* このレポートがデバイスを参照しているので以下のレポートにはDeviceが破棄されていないと出る */
	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
	debugDevice.Reset();
#endif
}



void App::TermWindow() {

	// ウィンドウ登録の解除
	if (m_hInst != nullptr) {
		UnregisterClass(m_windowTitle, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}


// ImGui用ウィンドウプロシージャ
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
		
		/* ウィンドウが破棄される場合に送信される */
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			break;
	}

	// ImGuiの処理を行う
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);


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


/* imguiの関数は以下に定義 */

// Imgui 初期化
bool App::Init_Imgui() {

	// ディスクリプタヒープの取得
	m_pHeapForImgui = CreateDescriptorHeapForImgui();

	if (m_pHeapForImgui == nullptr) {

		return false;
	}

	return true;
}


// Imgui用DescriptorHeapの生成
ComPtr<ID3D12DescriptorHeap> App::CreateDescriptorHeapForImgui() {

	ComPtr<ID3D12DescriptorHeap> ret;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  /* シェーダーから見えるように設定 */
	desc.NodeMask       = 0;                                          /* 複数のGPUはない */
	desc.NumDescriptors = 1;                                          /* ヒープの数は１つ */
	desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     /* CBV_SRV_UAVで定義 */

	HRESULT hr = m_pDevice->CreateDescriptorHeap(
		&desc, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));

	return ret;
}

// ディスクリプタヒープの参照処理
ComPtr<ID3D12DescriptorHeap> App::GetHeapForImgui() {

	return m_pHeapForImgui;
}


// Imguiの描画
void App::ImguiRender() {

	// 描画前処理（１つのウィンドウにNewFrameが３つ必要）
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ウィンドウの定義
	ImGui::Begin("Render Test Menu");
	ImGui::SetWindowSize(ImVec2(300, 400), ImGuiCond_::ImGuiCond_FirstUseEver);
	ImGui::End();

	ImGui::Render();
	m_pCmdList->SetDescriptorHeaps(1, m_pHeapForImgui.GetAddressOf());      // Imguiのヒープをセット
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pCmdList.Get());  // コマンドをセット
}

// Imguiの終了処理
void App::Term_Imgui() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}