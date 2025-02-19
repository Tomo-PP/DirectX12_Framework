
/****************************************************************
 * Include
 ****************************************************************/
#include "Object.h"


Object::Object(DirectX::XMFLOAT3 Position):
	Pos   (Position),
	m_VBV (),
	m_IBV ()
{ /* DO_NOTHING */ }


Object::~Object() {

	Term();
}


void Object::Init(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap) {

	
	

	// ���߂̈ʒu�Ɖ�]��ݒ�
	void* World = m_Transform[0].GetMapBuf();
	Transform* pWorld = reinterpret_cast<Transform*>(World);
	pWorld->World *= DirectX::XMMatrixTranslation(Pos.x, Pos.y, Pos.z);
}



void Object::Term() {

}



void Object::Update() {
	

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList) {

	// �e�N�X�`���E���_
	pCmdList->IASetVertexBuffers(0, 1, &m_VBV);   /* ���_�̃Z�b�g */
	pCmdList->IASetIndexBuffer(&m_IBV);           /* �C���f�b�N�X�̃Z�b�g */

	auto IndexSize = static_cast<uint32_t>(m_mesh.Indices.size());
	pCmdList->DrawIndexedInstanced(IndexSize, 1, 0, 0, 0);         /* ���b�V���̕`��R�}���h */
}

