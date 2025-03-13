
/****************************************************************
 * 固定値
 ****************************************************************/
#define MOUSE_SENSE  (1000)  // マウス感度

// マウスのボタン
#define MOUSE_BUTTON0 (0)    // 左クリック
#define MOUSE_BUTTON1 (1)    // 右クリック
#define MOUSE_BUTTON2 (2)    // ホイールクリック

#define MOVE_SPEED    (10) // カメラの平行移動速度

/****************************************************************
 * Include
 ****************************************************************/
#include "Camera.h"


Camera::Camera():
	eyePos    (Vector3(10.0f, 0.0f, 10.0f)),
	targetPos (Vector3(0.0f, 0.0f, 0.0f)),
	upward    (Vector3(0.0f, 1.0f, 0.0f)),
	fovY      (45.0f),
	Clip_near (10.0f),
	Clip_far  (1000.0f),
	aspect    (WINDOW_WIDTH / WINDOW_HEIGHT)
{ /* DO_NOTHING */ }


Camera::~Camera()
{ /* DO_NOTHING */ }


bool Camera::Init(ID3D12Device* pDevice, DescriptorManager* pHeap) {

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		if (!pHeap->CreateCBV(pDevice, pHeap->GetGlobalHeap(), &m_Transform[i], sizeof(CameraView))) {

			return false;
		}

		// カメラ行列の設定
		void* ptr = m_Transform[i].GetMapBuf();
		CameraView* pView = reinterpret_cast<CameraView*>(ptr);
		pView->View = DirectX::XMMatrixLookAtLH(eyePos, targetPos, upward);

		std::cout << "[RightVector]" << std::endl;
		std::cout << pView->View.r[0].m128_f32[0] << ", " << pView->View.r[1].m128_f32[0] << ", " << pView->View.r[2].m128_f32[0] << std::endl;
		std::cout << "[UpVector]" << std::endl;
		std::cout << pView->View.r[0].m128_f32[1] << ", " << pView->View.r[1].m128_f32[1] << ", " << pView->View.r[2].m128_f32[1] << std::endl;
		std::cout << "[ForwardVector]" << std::endl;
		std::cout << pView->View.r[0].m128_f32[2] << ", " << pView->View.r[1].m128_f32[2] << ", " << pView->View.r[2].m128_f32[2] << std::endl;
		std::cout << std::endl;
	}


	return true;
}


void Camera::Term() {

	// 定数バッファの破棄
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		m_Transform[i].Term();
	}

}


void Camera::Update(Input* input) {


	// カメラの回転（左クリック時のみ）
	if (input->HoldMouse(MOUSE_BUTTON2)) {

		void* ptr = m_Transform[0].GetMapBuf();
		CameraView* pView = reinterpret_cast<CameraView*>(ptr);
		Vector3 rot = input->GetMouseMove();
		Vector3 right = Vector3(pView->View.r[0].m128_f32[0], pView->View.r[0].m128_f32[1], pView->View.r[0].m128_f32[2]);
		Vector3 axis_Y = Vector3(pView->View.r[1].m128_f32[0], pView->View.r[1].m128_f32[1], pView->View.r[1].m128_f32[2]);

		CameraRotateAxis(axis_Y, -rot.x / MOUSE_SENSE);  // 横回転
		CameraRotateAxis(right, -rot.y / MOUSE_SENSE);   // 縦回転
	}

	void* ptr = m_Transform[0].GetMapBuf();
	CameraView* pView = reinterpret_cast<CameraView*>(ptr);
	// カメラの移動
	if (input->HoldKey(DIK_W)) {

		Vector3 moveDir = Vector3(pView->View.r[0].m128_f32[0], pView->View.r[0].m128_f32[1], pView->View.r[0].m128_f32[2]);
		CameraTransition(moveDir);
	}
	if (input->HoldKey(DIK_S)) {

		Vector3 moveDir = Vector3(pView->View.r[0].m128_f32[0], pView->View.r[0].m128_f32[1], pView->View.r[0].m128_f32[2]);
		CameraTransition(-moveDir);
	}
	if (input->HoldKey(DIK_A)) {

		// 右方向
		Vector3 moveDir = Vector3(pView->View.r[2].m128_f32[0], pView->View.r[2].m128_f32[1], pView->View.r[2].m128_f32[2]);
		CameraTransition(moveDir);

	}
	if (input->HoldKey(DIK_D)) {

		// 左方向
		Vector3 moveDir = Vector3(pView->View.r[2].m128_f32[0], pView->View.r[2].m128_f32[1], pView->View.r[2].m128_f32[2]);
		CameraTransition(-moveDir);

	}

	std::cout << "[RightVector]" << std::endl;
	std::cout << pView->View.r[0].m128_f32[0] << ", " << pView->View.r[0].m128_f32[1] << ", " << pView->View.r[0].m128_f32[2] << std::endl;
	std::cout << "[UpVector]" << std::endl;
	std::cout << pView->View.r[1].m128_f32[0] << ", " << pView->View.r[1].m128_f32[1] << ", " << pView->View.r[1].m128_f32[2] << std::endl;
	std::cout << "[ForwardVector]" << std::endl;
	std::cout << pView->View.r[2].m128_f32[0] << ", " << pView->View.r[2].m128_f32[1] << ", " << pView->View.r[2].m128_f32[2] << std::endl;
	std::cout << std::endl;
}


D3D12_GPU_VIRTUAL_ADDRESS Camera::GetVirtualAddress(size_t FrameIndex) const {

	return m_Transform[FrameIndex].GetVirtualAddress();
}


Vector3 Camera::GetEyePos() const{

	return eyePos;
}


Vector3 Camera::GetTargetPos() const {

	return targetPos;
}


Vector3 Camera::GetUpWard() const{

	return upward;
}


void Camera::CameraRotateAxis(Vector3 axis, float angle) {

	if (axis.x == 0 && axis.y == 0 && axis.z == 0 || angle == 0) {

		return;
	}

	// カメラの回転
	axis.Normalize();
	//auto rot = DirectX::XMMatrixRotationAxis(axis, angle);
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		void* ptr = m_Transform[i].GetMapBuf();
		CameraView* pView = reinterpret_cast<CameraView*>(ptr);
		pView->View *= DirectX::XMMatrixRotationAxis(axis, angle);
	}
}


void Camera::CameraRotateY(float angle) {

	// カメラの回転
	auto rot = DirectX::XMMatrixRotationY(angle);
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		void* ptr = m_Transform[i].GetMapBuf();
		CameraView* pView = reinterpret_cast<CameraView*>(ptr);
		pView->View *= rot;
	}
}


void Camera::CameraTransition(Vector3 trans) {

	// カメラの移動
	trans = trans / MOVE_SPEED;
	auto move = DirectX::XMMatrixTranslationFromVector(trans);
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		void* ptr = m_Transform[i].GetMapBuf();
		CameraView* pView = reinterpret_cast<CameraView*>(ptr);
		pView->View *= move;
	}
}
