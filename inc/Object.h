#pragma once
/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"
#include "ComPtr.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "SimpleMath.h"
#include "FileUtil.h"
#include "DescriptorManager.h"

/****************************************************************
 * クラス
 ****************************************************************/
class VerteBuffer;
class IndexBuffer;
class ConstantBuffer;
class Texture;
class DescriptorManaer;


/****************************************************************
 * Object クラス
 ****************************************************************/
class Object {

private:

	DirectX::XMFLOAT3          Pos;             /* 座標 */
	VertexBuffer               m_VB;            /* 頂点バッファ */
	IndexBuffer                m_IB;            /* インデックスバッファ */

	ConstantBuffer             m_Transform[2];  /* 変換行列 */
	ConstantBuffer             m_Light;         /* ライト */
	ConstantBuffer             m_Material;      /* マテリアル */
	Texture                    m_DiffuseMap;    /* ディフューズマップ */
	Texture                    m_NormalMap;     /* 法線マップ */

public:

	Object();

	Object(DirectX::XMFLOAT3 Position);

	~Object();

	
	/****************************************************************
	 * 初期化処理
	 ****************************************************************/
	virtual bool Init(
		ID3D12Device*       pDevice,
		ID3D12CommandQueue* pCmdQueue,
		DescriptorManager*  DespManager,
		std::wstring        filename);


	/****************************************************************
	 * 破棄処理
	 ****************************************************************/
	virtual void Term();


	/****************************************************************
	 * 更新処理
	 ****************************************************************/
	virtual void Update(uint32_t FrameIndex);


	/****************************************************************
	 * 描画処理
	 ****************************************************************/
	virtual void Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex);


	/****************************************************************
	 * 変換行列関連の関数
	 ****************************************************************/
	void ModelScaling(Vector3 Scale);      /* スケール変更 */

	void ModelRotation(float angle);       /* 回転 */

	void ModelTranslation(Vector3 trans);  /* 平行移動 */

};