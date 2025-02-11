#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>



/****************************************************************
 * MeshVertex 構造体（3Dモデルの頂点情報などを格納）
 ****************************************************************/
struct MeshVertex{

	DirectX::XMFLOAT3 Position;  /* 頂点座標 */
	DirectX::XMFLOAT2 TexCoord;  /* uv座標 */
	DirectX::XMFLOAT3 Normal;    /* 法線ベクトル */
	DirectX::XMFLOAT3 Tangent;   /* 接ベクトル */
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
};



/****************************************************************
 * Mesh クラス
 ****************************************************************/
class Mesh {

private:

	std::vector<MeshVertex> vertices;    /* 頂点情報 */
	std::vector<uint32_t>   indices;     /* 頂点インデックス情報 */
	uint32_t                materialId;  /* マテリアル番号 */

public:
	
	/****************************************************************
	 * メッシュの読み込み（ほかのクラスでの参照に使用）
	 ****************************************************************/
	bool LoadMesh(
		const wchar_t*          filename,   /* 読み込むファイルまでのパス */
		std::vector<Mesh>&      mesh,       /* 読み込み後のメッシュ情報の格納先 */
		std::vector<Material>&  material);  /* 読み込み後のマテリアル情報の格納先 */
};

