#pragma once
/****************************************************************
 * Include
 ****************************************************************/
#include <windows.h>
#undef max
#undef min

#include <cstdint>

#include <cassert>
#include <cmath>
#include <d3d12.h>
#include <dxgi1_4.h>

#include <SimpleMath.h>
#include <d3dcompiler.h>
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
#include "VertexTypes.h"
#include "FileUtil.h"
#include "Mesh.h"
#include "DescriptorManager.h"
#include "ComPtr.h"
#include "Object.h"
#include "Input.h"
#include "Camera.h"


// imgui用
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"



/****************************************************************
 * Linker（ライブラリの指定）
 ****************************************************************/
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3dcompiler.lib" )


/*****************************************************************
 * using namespace SimpleMath
 *****************************************************************/
using namespace DirectX::SimpleMath;


/****************************************************************
 * class 宣言
 ****************************************************************/
class DescriptorManager;
class Input;


/****************************************************************
 * App Class
 ****************************************************************/
class App {

private:

	/*****************************************************************
	 * フレームバッファ数（画面の数・静的に宣言）
	 *****************************************************************/
	static const uint32_t FrameCount = 2;


	/*****************************************************************
	 * ウィンドウ用メンバ変数 
	 *****************************************************************/
	
	HINSTANCE m_hInst;        /* インスタンスハンドル（このプログラム自体のインスタンス） */
	HWND      m_hWnd;         /* ウィンドウのインスタンス */
	uint32_t  m_Width;        /* ウィンドウの横幅 */
	uint32_t  m_Height;       /* ウィンドウの高さ */
	LPCWSTR   m_windowTitle;  /* ウィンドウの名前 */


	/*****************************************************************
	 * Direct3D用のメンバ変数
	 *****************************************************************/

	ComPtr<ID3D12Device>               m_pDevice;                    /* GPUデバイス */
	ComPtr<ID3D12CommandQueue>         m_pCmdQueue;                  /* コマンドキュー */
	ComPtr<IDXGISwapChain3>            m_pSwapChain;                 /* スワップチェイン（SwapChain3でフレームバッファ番号を取得できる） */
	ComPtr<ID3D12CommandAllocator>     m_pCmdAllocator[FrameCount];  /* コマンドアロケーター、フレーム数分必要（今回はダブルバッファリングなので２個） */
	ComPtr<ID3D12GraphicsCommandList>  m_pCmdList;                   /* コマンドリスト */
	ComPtr<ID3D12Fence>                m_pFence;                     /* フェンス */


	HANDLE                       m_FenceEvent;                 /* フェンスイベント（ウィンドウズの OSの機構を利用）*/
	uint64_t                     m_FenceCounter[FrameCount];   /* フェンスカウンター（この値がインクリメントされたらGPU処理が完了）*/
	uint32_t                     m_FrameIndex;                 /* 表示しているフレームバッファの番号 */


	/*****************************************************************
	 * 描画用インターフェイスのメンバ変数
	 *****************************************************************/
	ComPtr<ID3D12RootSignature>  m_pRootSignature;      /* ルートシグネチャ */
	ComPtr<ID3D12PipelineState>  m_pPSO;                /* パイプラインステート */
	DescriptorManager            m_DespManager;         /* ディスクリプタヒープの管理クラス */
	
	D3D12_VIEWPORT                m_Viewport;           /* ビューポート */
	D3D12_RECT                    m_Scissor;            /* シザー矩形 */


	/*****************************************************************
	 * オブジェクト
	 *****************************************************************/
	Object  model[5];
	Input input;
	Camera camera;


	/*****************************************************************
	 * Imgui用変数や関数
	 *****************************************************************/
	ComPtr<ID3D12DescriptorHeap> m_pHeapForImgui;  // ヒープ保持用

	bool Init_Imgui();                             // imguiの初期化

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeapForImgui();  // imguiのディスクリプタヒープの作成
	ComPtr<ID3D12DescriptorHeap> GetHeapForImgui();               // 他から参照できるようにする関数

	// Imguiの描画
	void ImguiRender();

	// Imguiの終了処理
	void Term_Imgui();

public:


	App(uint32_t width, uint32_t height, LPCWSTR title);

	~App();


	/*****************************************************************
	 * アプリケーションの実行処理
	 *****************************************************************/
	void Run();


private:

	/*****************************************************************
	 * アプリ全体の初期化処理
	 *****************************************************************/
	bool InitApp();


	/*****************************************************************
	 * ウィンドウの初期化
	 *****************************************************************/
	bool InitWindow();


	/*****************************************************************
	 * Direct3Dの初期化
	 *****************************************************************/
	bool InitDirect3D();


	/*****************************************************************
	 * 描画用インターフェイスの初期化
	 *****************************************************************/
	bool OnInit();


	/*****************************************************************
	 * メインループ
	 *****************************************************************/
	void MainLoop();


	/*****************************************************************
	 * 更新処理
	 *****************************************************************/
	void Update();


	/*****************************************************************
	 * 描画処理
	 *****************************************************************/
	void Render();

	
	/*****************************************************************
	 * 画面の表示処理・次フレームの準備処理
	 *****************************************************************/
	void Present(uint32_t interval);


	/*****************************************************************
	 * GPUの描画完了を待機する処理
	 *****************************************************************/
	void WaitGPU();


	/*****************************************************************
	 * アプリケーションの破棄
	 *****************************************************************/
	void TermApp();


	/*****************************************************************
	 * ウィンドウの破棄
	 *****************************************************************/
	void TermWindow();


	/*****************************************************************
	 * Direct3Dの破棄
	 *****************************************************************/
	void TermDirect3D();


	/*****************************************************************
	 * 描画用インターフェースの破棄
	 *****************************************************************/
	void OnTerm();


};


/*****************************************************************
 * ウィンドウプロシージャ
 *****************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);


/*****************************************************************
 * インターフェースの解放（nullptrであるかを確認したのちに解放） 
 *****************************************************************/
template<typename T>
void SafeRelease(T* ptr);