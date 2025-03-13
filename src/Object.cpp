
/****************************************************************
 * Include
 ****************************************************************/
#include "Object.h"
#include "define.h"
#include <iostream>
#include "StrUtil.h"
#include "Quaternion.h"
#include <math.h>


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
	if (!m_VB.Init(pDevice, &resMesh[0])) {

		return false;
	}
	if (!m_IB.Init(pDevice, &resMesh[0])) {

		return false;
	}


	// カメラ用CBVの生成
	auto srcHeapHandle = pDespManager->GetGlobalHeap()->GetCPUDescriptorHandleForHeapStart();
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		srcHeapHandle.ptr += incrementSize * i;
		if (!pDespManager->CopyDescriptorHeap(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), srcHeapHandle)) {

			return false;
		}
	}


	// CBVの生成
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Transform[i], sizeof(Transform))) {

			return false;
		}

		constexpr auto fovY   = DirectX::XMConvertToRadians(37.5f);                           /* カメラの Y軸に対する画角 */
		auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);   /* 高さに対する幅の割合 */
		
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* transform = reinterpret_cast<Transform*>(ptr);
		transform->World      = DirectX::XMMatrixIdentity();
		transform->Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, 1.0f, 1000.0f);
	}



	// ライトの生成
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Light, sizeof(Light))) {

		return false;
	}
	void* ptr = m_Light.GetMapBuf();
	Light* light = reinterpret_cast<Light*>(ptr);
	light->LightPosition = Vector4(10.0f, 0.0f, 10.0f, 0.0f);
	light->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);



	// マテリアルの生成
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Material, sizeof(Material))) {

		return false;
	}
	ptr = m_Material.GetMapBuf();
	Material* material = reinterpret_cast<Material*>(ptr);
	material->Diffuse    = resMaterial[0].Diffuse;
	material->alpha      = resMaterial[0].alpha;
	material->Specular   = resMaterial[0].Specular;
	material->Shininess  = resMaterial[0].Shininess;
	//std::cout << resMaterial[0].DiffuseMap << std::endl;
	//std::cout << resMaterial[0].NormalMap << std::endl;
	std::wstring diffusePath = MbstrToWstr(resMaterial[0].DiffuseMap.c_str());
	std::wstring normalPath  = MbstrToWstr(resMaterial[0].NormalMap.c_str());


	// テクスチャの生成
	DirectX::ResourceUploadBatch batch(pDevice);
	batch.Begin();                                        // 内部処理でコマンドアロケーターとコマンドリストが生成される
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_DiffuseMap, diffusePath.c_str(), batch)) {

		return false;
	}
	
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_NormalMap, normalPath.c_str(), batch)) {

		return false;
	}

	resMesh.clear();
	resMaterial.clear();

	// コマンドの実行
	auto future = batch.End(pCmdQueue);

	// コマンド完了まで待機
	future.wait();

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



void Object::Update(uint32_t FrameIndex) {
	
	ModelRotation(0.01f);

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex) {

	// テクスチャ・頂点
	pCmdList->SetGraphicsRootConstantBufferView(1, m_Transform[FrameIndex].GetVirtualAddress()); /* 変換行列のセット */
	pCmdList->SetGraphicsRootConstantBufferView(2, m_Light.GetVirtualAddress());                 /* ライトをセット */
	pCmdList->SetGraphicsRootConstantBufferView(3, m_Material.GetVirtualAddress());              /* マテリアルのセット */
	pCmdList->SetGraphicsRootDescriptorTable(4, m_DiffuseMap.GetHandleGPU());                    /* ディフューズマップのセット */
	pCmdList->SetGraphicsRootDescriptorTable(5, m_NormalMap.GetHandleGPU());                     /* 法線マップのセット */
	auto VBV = m_VB.GetVBV();
	auto IBV = m_IB.GetIBV();
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &VBV);                                                     /* 頂点のセット */
	pCmdList->IASetIndexBuffer(&IBV);                                                             /* インデックスのセット */
	pCmdList->DrawIndexedInstanced(m_IB.GetIndexNum(), 1, 0, 0, 0);                               /* メッシュの描画コマンド */
}


/****************************************************************
 * 変換行列関連
 ****************************************************************/
void Object::ModelScaling(Vector3 Scale) {

	auto ScaleMatrix = DirectX::XMMatrixScalingFromVector(Scale);

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		// マッピング済みのバッファを変換
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= ScaleMatrix;
	}

}


void Object::ModelRotation(float angle) {

	auto RotateMatrix = DirectX::XMMatrixRotationX(angle);

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		// マッピング済みのバッファを変換
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= RotateMatrix;
	}

}


void Object::ModelQuaternion(float angle, Vector3 axis, Vector3 rot) {

	// 回転軸の設定
	float radian = angle * (M_PI / 180);
	axis.Normalize();

	// クオータニオンの生成
	float th = sin(radian / 2);
	QuatLib::Quaternion quat = QuatLib::Quaternion(
		axis.x * th,
		axis.y * th,
		axis.z * th,
		cos(radian / 2)
	);



}


void Object::ModelTranslation(Vector3 trans) {

	auto TransMatrix = DirectX::XMMatrixTranslationFromVector(trans);

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		//マッピング済みのバッファを変換
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= TransMatrix;
	}



}