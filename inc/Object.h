#pragma once
/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <DirectXMath.h>
#include "Mesh.h"
#include "ComPtr.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "SimpleMath.h"


/****************************************************************
 * Object クラス
 ****************************************************************/
class Object {

private:

	DirectX::XMFLOAT3          Pos;             /* 座標 */
	Mesh                       m_mesh;          /* メッシュ情報 */
	D3D12_VERTEX_BUFFER_VIEW   m_VBV;           /* 頂点バッファビュー */
	D3D12_INDEX_BUFFER_VIEW    m_IBV;           /* インデックスバッファビュー */
	ConstantBuffer             m_Transform[2];  /* 変換行列 */
	ConstantBuffer             m_Light;         /* ライト */
	ConstantBuffer             m_Material;      /* マテリアル */
	Texture                    m_DiffuseMap;    /* ディフューズマップ */
	Texture                    m_NormalMap;     /* 法線マップ */

public:

	Object(DirectX::XMFLOAT3 Position);

	~Object();

	
	/****************************************************************
	 * 初期化処理
	 ****************************************************************/
	virtual void Init(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap);


	/****************************************************************
	 * 破棄処理
	 ****************************************************************/
	virtual void Term();


	/****************************************************************
	 * 更新処理
	 ****************************************************************/
	virtual void Update();


	/****************************************************************
	 * 描画処理
	 ****************************************************************/
	virtual void Render(ID3D12GraphicsCommandList* pCmdList);
};