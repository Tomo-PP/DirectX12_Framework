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
 * テクスチャのビュー 構造体
 ****************************************************************/
struct TextureView {

	ComPtr<ID3D12Resource>      pResource;  /* リソース */
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;  /* CPUのディスクリプタに対するハンドル */
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;  /* GPUのディスクリプタに対するハンドル */
};

struct LightBuffer {

	Vector4 LightPosition;
	Color   LightColor;
};


/****************************************************************
 * 定数バッファビュー 構造体
 ****************************************************************/
template<typename T>
struct ConstantBufferView {

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;    /* 定数バッファの設定 */
	D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;  /* CPUディスクリプタハンドル */
	D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;  /* GPUディスクリプタハンドル */
	T*                              pBuffer;    /* バッファ先頭を指すポインタ */
};


/****************************************************************
 * class 宣言
 ****************************************************************/
class DescriptorManager;


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
	 * Descriptor関連
	 *****************************************************************/
	DescriptorManager             m_DespManager;               /* ディスクリプタヒープの管理クラス */

	 /* VBVと IBVは特殊 */
	D3D12_VERTEX_BUFFER_VIEW      m_VBV;                             /* 頂点バッファビュー（１モデルの頂点の塊に関する情報・モデルの数分必要）*/
	D3D12_INDEX_BUFFER_VIEW       m_IBV;                             /* インデックスバッファビュー */

	/* Heap */
	ComPtr<ID3D12DescriptorHeap>       m_pHeapCBV_SRV_UAV;           /* ディスクリプタヒープ（定数バッファビュー・シェーダーリソースバッファビュー・アンオーダーアクセスビュー） */
	
	ComPtr<ID3D12Resource>             m_pVB;                        /* 頂点バッファ */
	ComPtr<ID3D12Resource>             m_pIB;                        /* インデックスバッファ */

	ComPtr<ID3D12Resource>             m_pCB[4];        /* 定数バッファ（変換行列×２＋ライトバッファ＋マテリアル＝４）*/


	ConstantBufferView<Transform> m_CBV[FrameCount];      /* 定数バッファビュー（バックバッファの数×モデルの数分必要）*/
	ConstantBufferView<Material>  m_Material;             /* マテリアルのデータを保存 */
	TextureView                   m_Texture;              /* テクスチャのデータを保存 */
	TextureView                   NormalMapSRV;           /* 法線マップのテクスチャデータを保存 */

	/*****************************************************************
	 * 描画用インターフェイスのメンバ変数
	 *****************************************************************/
	ComPtr<ID3D12RootSignature>  m_pRootSignature;      /* ルートシグネチャ */
	ComPtr<ID3D12PipelineState>  m_pPSO;                /* パイプラインステート */
	
	D3D12_VIEWPORT                m_Viewport;              /* ビューポート */
	D3D12_RECT                    m_Scissor;               /* シザー矩形 */


	/*****************************************************************
	 * メッシュ用変数
	 *****************************************************************/
	std::vector<Mesh>        m_meshes;     /* メッシュ情報（複数定義できるように可変配列）*/
	std::vector<Material>    m_materials;  /* マテリアル情報 */


	/*****************************************************************
	 * ライト用変数
	 *****************************************************************/
	ConstantBufferView<LightBuffer> m_LightCBV;


	/*****************************************************************
	 * オブジェクト
	 *****************************************************************/
	Object  model;


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