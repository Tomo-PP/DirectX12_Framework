#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <dinput.h>
#include <d3d12.h>
#include <SimpleMath.h>

/****************************************************************
 * リンカ設定
 ****************************************************************/
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


/****************************************************************
 * using namespace
 ****************************************************************/
using namespace DirectX::SimpleMath;


/****************************************************************
 * Input クラス
 ****************************************************************/
class Input {

private:

	LPDIRECTINPUT8        inputDevice;     /* DirectInput オブジェクトのポインタ */
	LPDIRECTINPUTDEVICE8  keyboardDevice;  /* keyboard デバイス */
	LPDIRECTINPUTDEVICE8  mouseDevice;     /* mouse    デバイス */

	BYTE                  NowKeyBuf[256];  /* 現在のキーボードの状態 */
	BYTE                  PreKeyBuf[256];  /* 1フレーム前のキーボードの状態 */
	DIMOUSESTATE          NowMouseBuf;     /* 現在のマウスの状態 */
	DIMOUSESTATE          PreMouseBuf;     /* １フレーム前のマウスの状態 */

public:

	Input();

	~Input();


	/****************************************************************
	 * DirectInputの初期化
	 ****************************************************************/
	bool InitDirectInput(HINSTANCE hInst);


	/****************************************************************
	 * キーボードデバイスの初期化
	 ****************************************************************/
	bool InitKeyBoard(HWND hWnd);


	/****************************************************************
	 * マウスデバイスの初期化
	 ****************************************************************/
	bool InitMouse(HWND hWnd);


	/****************************************************************
	 * 入力デバイスの破棄
	 ****************************************************************/
	void TermInputDevice();


	/****************************************************************
	 * 入力の更新処理
	 ****************************************************************/
	void Update();



	/****************************************************************
	 * キーボードを押した瞬間
	 ****************************************************************/
	bool PressKey(const int idx) const;


	/****************************************************************
	 * キーボードを押している間
	 ****************************************************************/
	bool HoldKey(const int idx) const;


	/****************************************************************
	 * キーボードを押している間
	 ****************************************************************/
	bool ReleaseKey(const int idx) const;


	/****************************************************************
	 * マウスを押している間
	 ****************************************************************/
	bool PressMouse(const int idx) const;


	/****************************************************************
	 * マウスを押している間
	 ****************************************************************/
	bool HoldMouse(const int idx) const;


	/****************************************************************
	 * マウスを押している間
	 ****************************************************************/
	bool ReleaseMouse(const int idx) const;


	/****************************************************************
	 * マウスの移動座標取得
	 ****************************************************************/
	Vector3 GetMouseMove() const;

};




