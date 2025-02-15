
/*****************************************************************
 * Include
 *****************************************************************/
#include "Window.h"
#include <iostream>

Window::Window():
	m_hInst       (nullptr),
	m_hWnd        (nullptr),
	m_Width       (0),
	m_Height      (0),
	m_windowTitle (nullptr)
{ /* DO_NOTHING */ }


Window::~Window(){

	TermWindow();
}


bool Window::InitWindow(uint32_t width, uint32_t height, LPCWSTR title) {

	// インスタンスハンドルの取得（このプログラム自体のインスタンス）
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		std::cout << "Failed to Get Instance Handle." << std::endl;
		return false;
	}

	// ウィンドウの設定
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);                  /* 構造体のサイズ */
	windowClass.style = CS_HREDRAW | CS_VREDRAW;             /* ウィンドウクラスのスタイル（ウィンドウの枠線などの設定）*/
	//windowClass.lpfnWndProc = WndProc;                             /* ウィンドウプロシージャ */
	windowClass.hIcon = LoadIcon(hInst, IDI_APPLICATION);    /* ウィンドウのアイコンのハンドル設定 */
	windowClass.hCursor = LoadCursor(hInst, IDC_ARROW);        /* ウィンドウのマウスカーソル設定 */
	windowClass.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);  /* ウィンドウの背景を描画するブラシへのハンドル */
	windowClass.lpszMenuName = nullptr;                             /* メニューバー表示の際に設定（今回は使用しないので nullptr ）*/
	windowClass.lpszClassName = m_windowTitle;                       /* ウィンドウのタイトル */
	windowClass.hIconSm = LoadIcon(hInst, IDI_APPLICATION);    /* ウィンドウのアイコンの設定 */

	// ウィンドウの登録
	if (!RegisterClassEx(&windowClass)) {
		return false;
	}


	// インスタンスハンドルを設定（ポインタの受け渡し）
	m_hInst = hInst;


	// ウィンドウサイズの設定（左と上がそれぞれ０の基準）
	RECT rect = {};
	rect.right = m_Width;   /* 横幅 */
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



void Window::TermWindow() {

	// ウィンドウ登録の解除
	if (m_hInst != nullptr) {
		UnregisterClass(m_windowTitle, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}



//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//
//	switch (msg) {
//
//		/* ウィンドウが破棄される場合に送信される */
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//
//	default:
//		break;
//	}
//
//	/* 他のメッセージを処理してくれる関数を返す */
//	return DefWindowProc(hwnd, msg, wp, lp);
//}