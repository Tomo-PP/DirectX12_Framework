
/****************************************************************
 * Include
 ****************************************************************/
#include "Input.h"
#include <iostream>


/****************************************************************
 * �Œ�l
 ****************************************************************/
#define KEYPUSH (0x80)


Input::Input() :
	inputDevice    (nullptr),
	keyboardDevice (nullptr),
	mouseDevice    (nullptr)
{ 
	// �o�b�t�@�̏�����
	memset(NowKeyBuf, 0, sizeof(NowKeyBuf));
	memset(PreKeyBuf, 0, sizeof(PreKeyBuf));
	memset(&NowMouseBuf, 0, sizeof(NowMouseBuf));
	memset(&PreMouseBuf, 0, sizeof(PreMouseBuf));
}


Input::~Input()
{ /* DO_NOTHING */ }


bool Input::InitDirectInput(HINSTANCE hInst) {

	HRESULT hr = DirectInput8Create(
		hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&inputDevice,
		nullptr);
	if (FAILED(hr)) {

		std::cout << "Input Error : Can't Create DirectInput8 Device." << std::endl;
		return false;
	}

	return true;
}


bool Input::InitKeyBoard(HWND hWnd) {

	HRESULT hr = inputDevice->CreateDevice(GUID_SysKeyboard, &keyboardDevice, nullptr);
	if (FAILED(hr)) {

		std::cout << "KeyBorad Error : Failed to Create KeyBoardDevice." << std::endl;
		return false;
	}

	// �t�H�[�}�b�g�ݒ�
	hr = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) {

		std::cout << "KeyBorad Error : Failed to set KeyBoardDevice Format." << std::endl;
		return false;
	}

	// �������x���i�f�o�C�X�̐�L���j
	hr = keyboardDevice->SetCooperativeLevel(
		hWnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {

		std::cout << "KeyBoard Error : Failed to set CooperativeLevel." << std::endl;
		return false;
	}

	// ����I��
	return true;
}


bool Input::InitMouse(HWND hWnd) {

	HRESULT hr = inputDevice->CreateDevice(GUID_SysMouse, &mouseDevice, nullptr);
	if (FAILED(hr)) {

		std::cout << "Mouse Error : Failed to Create Mouse Device." << std::endl;
		return false;
	}

	// �t�H�[�}�b�g�̐ݒ�
	hr = mouseDevice->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr)) {

		std::cout << "Mouse Error : Failed to set MouseDevice Format." << std::endl;
		return false;
	}

	// �������x���i�f�o�C�X�̐�L���j
	hr = mouseDevice->SetCooperativeLevel(
		hWnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {

		std::cout << "Mouse Error : Failed to set CooperativeLevel." << std::endl;
		return false;
	}

	// ����I��
	return true;
}


void Input::TermInputDevice() {

	if (keyboardDevice != nullptr) {

		keyboardDevice->Unacquire();
		keyboardDevice = nullptr;
	}

	if (mouseDevice != nullptr) {

		mouseDevice->Unacquire();
		mouseDevice = nullptr;
	}

	if (inputDevice != nullptr) {

		inputDevice->Release();
		inputDevice = nullptr;
	}
}


void Input::Update() {

	// �L�[�E�}�E�X�̍X�V
	memcpy(PreKeyBuf, NowKeyBuf, sizeof(PreKeyBuf));
	PreMouseBuf = NowMouseBuf;


	// �L�[�{�[�h���͂̎擾
	if (SUCCEEDED(keyboardDevice->Acquire())) {

		keyboardDevice->GetDeviceState(sizeof(NowKeyBuf), (LPVOID)&NowKeyBuf);
	}

	// �}�E�X�̓��͂��擾
	if (SUCCEEDED(mouseDevice->Acquire())) {

		mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &NowMouseBuf);
	}

	//printf("MousePos : (%ld, %ld, %ld)\n", NowMouseBuf.lX, NowMouseBuf.lY, NowMouseBuf.lZ);

}



bool Input::PressKey(const int idx) const {

	if (!(PreKeyBuf[idx] & KEYPUSH) && (NowKeyBuf[idx] & KEYPUSH)){

		return true;
	}

	return false;
}


bool Input::HoldKey(const int idx) const {

	if ((!(PreKeyBuf[idx] & KEYPUSH) && (NowKeyBuf[idx] & KEYPUSH)) || 
		( (PreKeyBuf[idx] & KEYPUSH) && (NowKeyBuf[idx] & KEYPUSH))) {

		return true;
	}

	return false;
}


bool Input::ReleaseKey(const int idx) const {

	if ((PreKeyBuf[idx] & KEYPUSH) && !(NowKeyBuf[idx] & KEYPUSH)) {

		return true;
	}

	return false;
}


bool Input::PressMouse(int idx) const {

	if (!(PreMouseBuf.rgbButtons[idx] & KEYPUSH) && (NowMouseBuf.rgbButtons[idx] & KEYPUSH)) {

		return true;
	}
	return false;
}


bool Input::HoldMouse(int idx) const {

	if ((!(PreMouseBuf.rgbButtons[idx] & KEYPUSH) && (NowMouseBuf.rgbButtons[idx] & KEYPUSH)) ||
		 ((PreMouseBuf.rgbButtons[idx] & KEYPUSH) && (NowMouseBuf.rgbButtons[idx] & KEYPUSH))) {

		return true;
	}
	return false;
}


bool Input::ReleaseMouse(int idx) const {

	if ((PreMouseBuf.rgbButtons[idx] & KEYPUSH) && !(NowMouseBuf.rgbButtons[idx] & KEYPUSH)) {

		return true;
	}

	return false;
}


Vector3 Input::GetMouseMove() const{

	return Vector3(NowMouseBuf.lX, NowMouseBuf.lY, NowMouseBuf.lZ);
}