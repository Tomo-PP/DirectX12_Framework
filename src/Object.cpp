
/****************************************************************
 * Include
 ****************************************************************/
#include "Object.h"
#include "define.h"
#include <iostream>


Object::Object():
	Pos  (DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))
{ /* DO_NOTHING */ }


Object::Object(DirectX::XMFLOAT3 Position):
	Pos   (Position)
{ /* DO_NOTHING */ }


Object::~Object() {

	Term();
}


bool Object::Init(
	ID3D12Device*       pDevice,
	ID3D12CommandQueue* pCmdQueue,
	DescriptorManager*  pDespManager,
	std::wstring        filename)
{

	// メッシュの情報
	std::vector<Mesh>     resMesh;
	std::vector<Material> resMaterial;

	std::wstring ModelPath;
	if (!SearchFilePath(filename.c_str(), ModelPath)) {

		std::wcout << "Object Error : Can't find file " << filename << std::endl;
		return false;
	}
	if (!LoadMesh(ModelPath.c_str(), resMesh, resMaterial)) {

		return false;
	}


	// VB・IBの生成
	m_VB.Init(pDevice, &resMesh[0]);
	m_IB.Init(pDevice, &resMesh[0]);

	std::cout << "CBV開始" << std::endl;
	// CBVの生成
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Transform[i], sizeof(Transform))) {

			return false;
		}

		// ConstantBufferクラスで変換行列の初期化を定義する
		auto eyePos    = DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);    /* カメラ座標 */
		auto targetPos = DirectX::XMVectorZero();                          /* 注視点座標（原点）*/
		auto upward    = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);     /* カメラの高さ */

		constexpr auto fovY   = DirectX::XMConvertToRadians(37.5f);                                     /* カメラの Y軸に対する画角 */
		auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);   /* 高さに対する幅の割合 */
		
		void* ptr = m_Transform[0].GetMapBuf();
		Transform* transform = reinterpret_cast<Transform*>(ptr);
		transform->World      = DirectX::XMMatrixIdentity();
		transform->View       = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		transform->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
	}


	// ライトの生成
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Light, sizeof(Light))) {

		return false;
	}
	void* ptr = m_Light.GetMapBuf();
	Light* light = reinterpret_cast<Light*>(ptr);
	light->LightPosition = Vector4(0.0f, 50.0f, 0.0f, 0.0f);
	light->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);



	// マテリアルの生成
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Material, sizeof(Material))) {

		return false;
	}
	ptr = m_Transform[0].GetMapBuf();
	Material* material = reinterpret_cast<Material*>(ptr);
	material->Diffuse    = resMaterial[0].Diffuse;
	material->alpha      = resMaterial[0].alpha;
	material->Specular   = resMaterial[0].Specular;
	material->Shininess  = resMaterial[0].Shininess;



	// テクスチャの生成
	DirectX::ResourceUploadBatch batch(pDevice);
	batch.Begin();                                        // 内部処理でコマンドアロケーターとコマンドリストが生成される
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_DiffuseMap, L"Floor/BrickRound.dds", batch)) {

		return false;
	}
	
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_NormalMap, L"Floor/BrickRoundBUMP.dds", batch)) {

		return false;
	}

	resMesh.clear();
	resMaterial.clear();

	// コマンドの実行
	auto future = batch.End(pCmdQueue);

	// コマンド完了まで待機
	future.wait();


	// 初めの位置と回転を設定
	for (auto i = 0; i < _countof(m_Transform); i++) {

		void* World = m_Transform[0].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(World);
		pWorld->World *= DirectX::XMMatrixTranslation(Pos.x, Pos.y, Pos.z);
	}

	return true;
}



void Object::Term() {

	// テクスチャの破棄
	m_DiffuseMap.Term();
	m_NormalMap.Term();

	// 定数バッファの破棄
	for (auto i = 0; i < _countof(m_Transform); i++) {

		m_Transform[i].Term();
	}
	m_Light.Term();
	m_Material.Term();

	// 頂点バッファとインデックスバッファの破棄
	m_VB.Term();
	m_IB.Term();


	// 変数
	Pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}



void Object::Update() {
	

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex) {

	// テクスチャ・頂点
	auto VB = m_VB.GetVBV();
	auto IB = m_IB.GetIBV();
	pCmdList->IASetVertexBuffers(0, 1, &VB);                                                     /* 頂点のセット */
	pCmdList->IASetIndexBuffer(&IB);                                                             /* インデックスのセット */
	pCmdList->SetGraphicsRootConstantBufferView(0, m_Transform[FrameIndex].GetVirtualAddress()); /* 変換行列のセット */
	pCmdList->SetGraphicsRootConstantBufferView(1, m_Light.GetVirtualAddress());                 /* ライトをセット */
	pCmdList->SetGraphicsRootConstantBufferView(2, m_Material.GetVirtualAddress());              /* マテリアルのセット */
	pCmdList->SetGraphicsRootDescriptorTable(3, m_DiffuseMap.GetHandleGPU());                    /* ディフューズマップのセット */
	pCmdList->SetGraphicsRootDescriptorTable(4, m_NormalMap.GetHandleGPU());                     /* 法線マップのセット */
	pCmdList->DrawIndexedInstanced(m_IB.GetIndexNum(), 1, 0, 0, 0);                              /* メッシュの描画コマンド */
}

