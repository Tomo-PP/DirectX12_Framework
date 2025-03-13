
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

	// ���b�V���̏��
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


	// VB�EIB�̐���
	if (!m_VB.Init(pDevice, &resMesh[0])) {

		return false;
	}
	if (!m_IB.Init(pDevice, &resMesh[0])) {

		return false;
	}


	// �J�����pCBV�̐���
	auto srcHeapHandle = pDespManager->GetGlobalHeap()->GetCPUDescriptorHandleForHeapStart();
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		srcHeapHandle.ptr += incrementSize * i;
		if (!pDespManager->CopyDescriptorHeap(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), srcHeapHandle)) {

			return false;
		}
	}


	// CBV�̐���
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Transform[i], sizeof(Transform))) {

			return false;
		}

		constexpr auto fovY   = DirectX::XMConvertToRadians(37.5f);                           /* �J������ Y���ɑ΂����p */
		auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);   /* �����ɑ΂��镝�̊��� */
		
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* transform = reinterpret_cast<Transform*>(ptr);
		transform->World      = DirectX::XMMatrixIdentity();
		transform->Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, 1.0f, 1000.0f);
	}



	// ���C�g�̐���
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Light, sizeof(Light))) {

		return false;
	}
	void* ptr = m_Light.GetMapBuf();
	Light* light = reinterpret_cast<Light*>(ptr);
	light->LightPosition = Vector4(10.0f, 0.0f, 10.0f, 0.0f);
	light->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);



	// �}�e���A���̐���
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


	// �e�N�X�`���̐���
	DirectX::ResourceUploadBatch batch(pDevice);
	batch.Begin();                                        // ���������ŃR�}���h�A���P�[�^�[�ƃR�}���h���X�g�����������
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_DiffuseMap, diffusePath.c_str(), batch)) {

		return false;
	}
	
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_NormalMap, normalPath.c_str(), batch)) {

		return false;
	}

	resMesh.clear();
	resMaterial.clear();

	// �R�}���h�̎��s
	auto future = batch.End(pCmdQueue);

	// �R�}���h�����܂őҋ@
	future.wait();

	return true;
}



void Object::Term() {

	// �e�N�X�`���̔j��
	m_DiffuseMap.Term();
	m_NormalMap.Term();

	// �萔�o�b�t�@�̔j��
	for (auto i = 0; i < _countof(m_Transform); i++) {

		m_Transform[i].Term();
	}
	m_Light.Term();
	m_Material.Term();

	// ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�̔j��
	m_VB.Term();
	m_IB.Term();


	// �ϐ�
	Pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}



void Object::Update(uint32_t FrameIndex) {
	
	ModelRotation(0.01f);

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex) {

	// �e�N�X�`���E���_
	pCmdList->SetGraphicsRootConstantBufferView(1, m_Transform[FrameIndex].GetVirtualAddress()); /* �ϊ��s��̃Z�b�g */
	pCmdList->SetGraphicsRootConstantBufferView(2, m_Light.GetVirtualAddress());                 /* ���C�g���Z�b�g */
	pCmdList->SetGraphicsRootConstantBufferView(3, m_Material.GetVirtualAddress());              /* �}�e���A���̃Z�b�g */
	pCmdList->SetGraphicsRootDescriptorTable(4, m_DiffuseMap.GetHandleGPU());                    /* �f�B�t���[�Y�}�b�v�̃Z�b�g */
	pCmdList->SetGraphicsRootDescriptorTable(5, m_NormalMap.GetHandleGPU());                     /* �@���}�b�v�̃Z�b�g */
	auto VBV = m_VB.GetVBV();
	auto IBV = m_IB.GetIBV();
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &VBV);                                                     /* ���_�̃Z�b�g */
	pCmdList->IASetIndexBuffer(&IBV);                                                             /* �C���f�b�N�X�̃Z�b�g */
	pCmdList->DrawIndexedInstanced(m_IB.GetIndexNum(), 1, 0, 0, 0);                               /* ���b�V���̕`��R�}���h */
}


/****************************************************************
 * �ϊ��s��֘A
 ****************************************************************/
void Object::ModelScaling(Vector3 Scale) {

	auto ScaleMatrix = DirectX::XMMatrixScalingFromVector(Scale);

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		// �}�b�s���O�ς݂̃o�b�t�@��ϊ�
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= ScaleMatrix;
	}

}


void Object::ModelRotation(float angle) {

	auto RotateMatrix = DirectX::XMMatrixRotationX(angle);

	for (auto i = 0u; i < _countof(m_Transform); i++) {

		// �}�b�s���O�ς݂̃o�b�t�@��ϊ�
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= RotateMatrix;
	}

}


void Object::ModelQuaternion(float angle, Vector3 axis, Vector3 rot) {

	// ��]���̐ݒ�
	float radian = angle * (M_PI / 180);
	axis.Normalize();

	// �N�I�[�^�j�I���̐���
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

		//�}�b�s���O�ς݂̃o�b�t�@��ϊ�
		void* ptr = m_Transform[i].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(ptr);
		pWorld->World *= TransMatrix;
	}



}