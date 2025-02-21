#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <iostream>



/****************************************************************
 * MeshVertex 構造体（3Dモデルの頂点情報などを格納）
 ****************************************************************/
struct MeshVertex{

	DirectX::XMFLOAT3 Position;  /* 頂点座標 */
	DirectX::XMFLOAT2 TexCoord;  /* uv座標 */
	DirectX::XMFLOAT3 Normal;    /* 法線ベクトル */
	DirectX::XMFLOAT3 Tangent;   /* 接線ベクトル */

	MeshVertex() = default;

	MeshVertex(
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT2 texcoord,
		DirectX::XMFLOAT3 normal,
		DirectX::XMFLOAT3 tangent) :
	Position   (pos),
	TexCoord   (texcoord),
	Normal     (normal),
	Tangent    (tangent)
	{ /* DO_NOTHING */ };


	// シェーダーへの入力の形式（入力フォーマット配列とその配列の要素数）
	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementNum = 4;                                  /* 入力の数 */
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementNum];  /* 入力フォーマット */
};



/****************************************************************
 * Material 構造体（3Dモデルのマテリアル情報を格納）
 ****************************************************************/
struct Material {

	DirectX::XMFLOAT3 Diffuse;     /* 拡散反射率 */
	DirectX::XMFLOAT3 Specular;    /* 鏡面反射率 */
	float             alpha;       /* 透過成分 */
	float             Shininess;   /* 鏡面反射強度 */
	std::string       DiffuseMap;  /* テクスチャへのパス */
	std::string       NormalMap;   /* 法線マップへのパス */

	Material() :
		Diffuse    (DirectX::XMFLOAT3()),
		Specular   (DirectX::XMFLOAT3()),
		alpha      (0),
		Shininess  (0),
		DiffuseMap (),
		NormalMap  ()
	{ /* DO_NOTHING */ };
};



/****************************************************************
 * Mesh クラス
 ****************************************************************/
class Mesh {

public:

	std::vector<MeshVertex> Vertices;    /* 頂点情報 */
	std::vector<uint32_t>   Indices;     /* 頂点インデックス情報 */
	uint32_t                MaterialId;  /* マテリアル番号 */
};



/****************************************************************
 * メッシュの読み込み（ほかのクラスでの参照に使用）
 ****************************************************************/
bool LoadMesh(
	const wchar_t* filename,             /* 読み込むファイルまでのパス */
	std::vector<Mesh>& meshes,           /* 読み込み後のメッシュ情報の格納先 */
	std::vector<Material>& materials);   /* 読み込み後のマテリアル情報の格納先 */
