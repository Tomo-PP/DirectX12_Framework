
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
	m_VB.Init(pDevice, &resMesh[0]);
	m_IB.Init(pDevice, &resMesh[0]);

	std::cout << "CBV�J�n" << std::endl;
	// CBV�̐���
	for (auto i = 0u; i < _countof(m_Transform); i++) {

		if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Transform[i], sizeof(Transform))) {

			return false;
		}

		// ConstantBuffer�N���X�ŕϊ��s��̏��������`����
		auto eyePos    = DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);    /* �J�������W */
		auto targetPos = DirectX::XMVectorZero();                          /* �����_���W�i���_�j*/
		auto upward    = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);     /* �J�����̍��� */

		constexpr auto fovY   = DirectX::XMConvertToRadians(37.5f);                                     /* �J������ Y���ɑ΂����p */
		auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);   /* �����ɑ΂��镝�̊��� */
		
		void* ptr = m_Transform[0].GetMapBuf();
		Transform* transform = reinterpret_cast<Transform*>(ptr);
		transform->World      = DirectX::XMMatrixIdentity();
		transform->View       = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		transform->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
	}


	// ���C�g�̐���
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Light, sizeof(Light))) {

		return false;
	}
	void* ptr = m_Light.GetMapBuf();
	Light* light = reinterpret_cast<Light*>(ptr);
	light->LightPosition = Vector4(0.0f, 50.0f, 0.0f, 0.0f);
	light->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);



	// �}�e���A���̐���
	if (!pDespManager->CreateCBV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_Material, sizeof(Material))) {

		return false;
	}
	ptr = m_Transform[0].GetMapBuf();
	Material* material = reinterpret_cast<Material*>(ptr);
	material->Diffuse    = resMaterial[0].Diffuse;
	material->alpha      = resMaterial[0].alpha;
	material->Specular   = resMaterial[0].Specular;
	material->Shininess  = resMaterial[0].Shininess;



	// �e�N�X�`���̐���
	DirectX::ResourceUploadBatch batch(pDevice);
	batch.Begin();                                        // ���������ŃR�}���h�A���P�[�^�[�ƃR�}���h���X�g�����������
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_DiffuseMap, L"Floor/BrickRound.dds", batch)) {

		return false;
	}
	
	if (!pDespManager->CreateSRV(pDevice, pDespManager->GetHeapCBV_SRV_UAV(), &m_NormalMap, L"Floor/BrickRoundBUMP.dds", batch)) {

		return false;
	}

	resMesh.clear();
	resMaterial.clear();

	// �R�}���h�̎��s
	auto future = batch.End(pCmdQueue);

	// �R�}���h�����܂őҋ@
	future.wait();


	// ���߂̈ʒu�Ɖ�]��ݒ�
	for (auto i = 0; i < _countof(m_Transform); i++) {

		void* World = m_Transform[0].GetMapBuf();
		Transform* pWorld = reinterpret_cast<Transform*>(World);
		pWorld->World *= DirectX::XMMatrixTranslation(Pos.x, Pos.y, Pos.z);
	}

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



void Object::Update() {
	

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex) {

	// �e�N�X�`���E���_
	auto VB = m_VB.GetVBV();
	auto IB = m_IB.GetIBV();
	pCmdList->IASetVertexBuffers(0, 1, &VB);                                                     /* ���_�̃Z�b�g */
	pCmdList->IASetIndexBuffer(&IB);                                                             /* �C���f�b�N�X�̃Z�b�g */
	pCmdList->SetGraphicsRootConstantBufferView(0, m_Transform[FrameIndex].GetVirtualAddress()); /* �ϊ��s��̃Z�b�g */
	pCmdList->SetGraphicsRootConstantBufferView(1, m_Light.GetVirtualAddress());                 /* ���C�g���Z�b�g */
	pCmdList->SetGraphicsRootConstantBufferView(2, m_Material.GetVirtualAddress());              /* �}�e���A���̃Z�b�g */
	pCmdList->SetGraphicsRootDescriptorTable(3, m_DiffuseMap.GetHandleGPU());                    /* �f�B�t���[�Y�}�b�v�̃Z�b�g */
	pCmdList->SetGraphicsRootDescriptorTable(4, m_NormalMap.GetHandleGPU());                     /* �@���}�b�v�̃Z�b�g */
	pCmdList->DrawIndexedInstanced(m_IB.GetIndexNum(), 1, 0, 0, 0);                              /* ���b�V���̕`��R�}���h */
}

