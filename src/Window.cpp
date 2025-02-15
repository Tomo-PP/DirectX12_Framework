
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

	// �C���X�^���X�n���h���̎擾�i���̃v���O�������̂̃C���X�^���X�j
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		std::cout << "Failed to Get Instance Handle." << std::endl;
		return false;
	}

	// �E�B���h�E�̐ݒ�
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);                  /* �\���̂̃T�C�Y */
	windowClass.style = CS_HREDRAW | CS_VREDRAW;             /* �E�B���h�E�N���X�̃X�^�C���i�E�B���h�E�̘g���Ȃǂ̐ݒ�j*/
	//windowClass.lpfnWndProc = WndProc;                             /* �E�B���h�E�v���V�[�W�� */
	windowClass.hIcon = LoadIcon(hInst, IDI_APPLICATION);    /* �E�B���h�E�̃A�C�R���̃n���h���ݒ� */
	windowClass.hCursor = LoadCursor(hInst, IDC_ARROW);        /* �E�B���h�E�̃}�E�X�J�[�\���ݒ� */
	windowClass.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);  /* �E�B���h�E�̔w�i��`�悷��u���V�ւ̃n���h�� */
	windowClass.lpszMenuName = nullptr;                             /* ���j���[�o�[�\���̍ۂɐݒ�i����͎g�p���Ȃ��̂� nullptr �j*/
	windowClass.lpszClassName = m_windowTitle;                       /* �E�B���h�E�̃^�C�g�� */
	windowClass.hIconSm = LoadIcon(hInst, IDI_APPLICATION);    /* �E�B���h�E�̃A�C�R���̐ݒ� */

	// �E�B���h�E�̓o�^
	if (!RegisterClassEx(&windowClass)) {
		return false;
	}


	// �C���X�^���X�n���h����ݒ�i�|�C���^�̎󂯓n���j
	m_hInst = hInst;


	// �E�B���h�E�T�C�Y�̐ݒ�i���Əオ���ꂼ��O�̊�j
	RECT rect = {};
	rect.right = m_Width;   /* ���� */
	rect.bottom = m_Height;  /* ���� */


	// �E�B���h�E�T�C�Y�𒲐�
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rect, style, FALSE);


	// �E�B���h�E�𐶐�
	m_hWnd = CreateWindowEx(
		0,                       /* �g���E�B���h�E�̐ݒ�i����͎g�p���Ȃ��j*/
		m_windowTitle,           /* �E�B���h�E�N���X���̐ݒ� */
		m_windowTitle,           /* �E�B���h�E�̃^�C�g���o�[�ɕ\������閼�O�̐ݒ� */
		style,                   /* �E�B���h�E�̃X�^�C���ݒ�i���j���[�o�[�Ȃǂ̐ݒ�j*/
		CW_USEDEFAULT,           /* �E�B���h�E�̏��� x���W */
		CW_USEDEFAULT,           /* �E�B���h�E�̏��� y���W */
		rect.right - rect.left,  /* �E�B���h�E�� */
		rect.bottom - rect.top,  /* �E�B���h�E�̍��� */
		nullptr,                 /* �쐬����E�B���h�E�̐e�܂��̓I�[�i�[�E�B���h�E�̃n���h�����w�� */
		nullptr,                 /* �E�B���h�E�X�^�C���ɉ������q�E�B���h�EID�̎w��i����͎g�p���Ȃ��j*/
		m_hInst,                 /* �E�B���h�E�Ɋ֘A�t�����Ă���C���X�^���X�̃n���h����ݒ� */
		nullptr);                /* ��������E�B���h�E�ɓn���C�ӂ̃p�����[�^�[���w��i����͎g�p���Ȃ��j*/

	if (m_hWnd == nullptr) {
		return false;
	}


	// �E�B���h�E��\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);


	// �E�B���h�E�̍X�V
	UpdateWindow(m_hWnd);


	// �E�B���h�E�Ƀt�H�[�J�X
	SetFocus(m_hWnd);


	// ����I��
	return true;
}



void Window::TermWindow() {

	// �E�B���h�E�o�^�̉���
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
//		/* �E�B���h�E���j�������ꍇ�ɑ��M����� */
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//
//	default:
//		break;
//	}
//
//	/* ���̃��b�Z�[�W���������Ă����֐���Ԃ� */
//	return DefWindowProc(hwnd, msg, wp, lp);
//}