
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

	
	

	// 初めの位置と回転を設定
	void* World = m_Transform[0].GetMapBuf();
	Transform* pWorld = reinterpret_cast<Transform*>(World);
	pWorld->World *= DirectX::XMMatrixTranslation(Pos.x, Pos.y, Pos.z);
}



void Object::Term() {

}



void Object::Update() {
	

}


void Object::Render(ID3D12GraphicsCommandList* pCmdList) {

	// テクスチャ・頂点
	pCmdList->IASetVertexBuffers(0, 1, &m_VBV);   /* 頂点のセット */
	pCmdList->IASetIndexBuffer(&m_IBV);           /* インデックスのセット */

	auto IndexSize = static_cast<uint32_t>(m_mesh.Indices.size());
	pCmdList->DrawIndexedInstanced(IndexSize, 1, 0, 0, 0);         /* メッシュの描画コマンド */
}

