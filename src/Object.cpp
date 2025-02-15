
/****************************************************************
 * Include
 ****************************************************************/
#include "Object.h"


Object::Object():
	Pos   (DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))
{


}


Object::~Object() {

	Term();
}


void Object::Init() {


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

