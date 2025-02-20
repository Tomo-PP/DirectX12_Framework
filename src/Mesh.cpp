
/****************************************************************
 * Includes
 ****************************************************************/
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <codecvt>
#include <cassert>
#include <iostream>



/****************************************************************
 * 固定値
 ****************************************************************/
#define FORMAT_FLOAT3   DXGI_FORMAT_R32G32B32_FLOAT                 /* float型 4バイトのRGBフォーマット */
#define FORMAT_FLOAT2   DXGI_FORMAT_R32G32_FLOAT                    /* float型 4バイトのRGフォーマット */
#define APPEND_ALIGNED  D3D12_APPEND_ALIGNED_ELEMENT                /* 入力レイアウトが連結していることを表す */
#define IN_VERTEX_DATA  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA  /* 入力要素を頂点ごとデータとして扱うことを表す */


/****************************************************************
 * 頂点情報の入力レイアウト
 ****************************************************************/
const D3D12_INPUT_ELEMENT_DESC MeshVertex::InputElements[] = {

	{"POSITION", 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"TEXCOORD", 0, FORMAT_FLOAT2, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"NORMAL"  , 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"TANGENT" , 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0}
};

// 入力要素と入力の数を表す
const D3D12_INPUT_LAYOUT_DESC MeshVertex::InputLayout = {

	MeshVertex::InputElements,   /* 入力フォーマット配列 */
	MeshVertex::InputElementNum  /* 配列の要素数 */
};



/****************************************************************
 * wchar_t型 から char型（utf-8）に変換処理
 ****************************************************************/
std::string ToUTF8(const std::wstring& str) {

	// ワイド文字の長さをマルチバイトの長さに変換
	auto length = WideCharToMultiByte(CP_UTF8, 0u, str.data(), -1, nullptr, 0, nullptr, nullptr);


	// マルチバイト分の長さのcharを動的に取得
	auto buffer = new char[length];


	// ワイド文字をマルチバイトに変換
	WideCharToMultiByte(CP_UTF8, 0u, str.data(), -1, buffer, length, nullptr, nullptr);

	// 結果を格納
	std::string result(buffer);

	// 動的取得したバッファの開放
	delete[] buffer;
	buffer = nullptr;

	return result;
}



/****************************************************************
 * メッシュの読み込み・解析を行うクラス
 ****************************************************************/
namespace {

class MeshLoader {

public:

	MeshLoader();

	~MeshLoader();



	/****************************************************************
	 * メッシュの読み込み（初めて読み込む場合）
	 ****************************************************************/
	bool Load(
		const wchar_t*         filename,    /* 読み込みたいファイルのパス（ワイド文字の先頭ポインタ） */
		std::vector<Mesh>&     meshes,      /* メッシュ情報 */
		std::vector<Material>& materials);  /* マテリアル情報 */


private:

	/****************************************************************
	 * メッシュデータの解析
	 ****************************************************************/
	void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);



	/****************************************************************
	 * マテリアル情報の解析
	 ****************************************************************/
	void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);

};

	
MeshLoader::MeshLoader()
{ /* DO_NOTHING */ }

MeshLoader::~MeshLoader()
{ /* DO_NOTHING */ }



bool MeshLoader::Load(const wchar_t* filename, std::vector<Mesh>& meshes, std::vector<Material>& materials) {

	/* モデルのロード処理 */
	if (filename == nullptr) {

		std::cout << "ファイルパスがありません" << std::endl;
		return false;
	}

	// wchar_t型 から char型（UTF-8）に変換
	auto filePath = ToUTF8(filename);


	// 読み込み時のオプション設定
	Assimp::Importer importer;
	int ImportFlag = 0;
	ImportFlag |= aiProcess_Triangulate;               /* 全てのメッシュの全ての面データを三角形化する */
	ImportFlag |= aiProcess_PreTransformVertices;      /* ノードグラフを削除し、それらが持つローカル変換行列で全ての頂点を事前に変換 */
	ImportFlag |= aiProcess_CalcTangentSpace;          /* 読み込まれたメッシュの接線ベクトルと従接戦ベクトルを計算 */
	ImportFlag |= aiProcess_GenSmoothNormals;          /* メッシュのすべての頂点に対して法線べクトルを生成 */
	ImportFlag |= aiProcess_GenUVCoords;               /* 非UVマッピングを適切な座標チャンネルに変換 */
	ImportFlag |= aiProcess_RemoveRedundantMaterials;  /* 冗長または参照されていないマテリアルを検索し削除する */
	ImportFlag |= aiProcess_OptimizeMeshes;            /* メッシュ数を最適化する */

	// ファイルを読み込む
	auto pScene = importer.ReadFile(filePath, ImportFlag);


	// 読み込み失敗かを確認
	if (pScene == nullptr) {
		std::cout << "モデルファイルが読み込めません" << std::endl;
		return false;
	}


	// メッシュのメモリを事前に確保
	meshes.clear();
	meshes.resize(pScene->mNumMeshes);  // メッシュの数だけ確保
	std::cout << "NumMesh:" << meshes.size() << std::endl;

	// メッシュデータを変換する
	for (auto i = 0u; i < meshes.size(); i++) {

		const auto pMesh = pScene->mMeshes[i];  // メッシュを読み込む
		ParseMesh(meshes[i], pMesh);            // メッシュを分割
	}


	// マテリアルのメモリを事前に確保
	materials.clear();
	materials.resize(pScene->mNumMaterials);  // メッシュの数だけ確保

	// マテリアルデータを変換する
	for (auto i = 0u; i < materials.size(); i++) {

		auto pMaterial = pScene->mMaterials[i];  // マテリアル情報を読み込む
		ParseMaterial(materials[i], pMaterial);  // マテリアルを分割
	}

	// シーンを破棄
	pScene = nullptr;

	std::cout << "モデル読み込み完了" << std::endl;
	// 正常終了
	return true;
}



void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh) {

	/* 頂点情報について */
	
	// マテリアル番号を設定
	dstMesh.MaterialId = pSrcMesh->mMaterialIndex;

	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	// 頂点数だけ事前確保
	dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
	std::cout << "NumVertices : " << pSrcMesh->mNumVertices << std::endl;

	for (auto i = 0u; i < pSrcMesh->mNumVertices; i++) {

		auto pPosition = &(pSrcMesh->mVertices[i]);                 /* 座標 */
		auto pNormal   = &(pSrcMesh->mNormals[i]);                  /* 法線ベクトル */
		auto pTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;  /* uv座標 */
		auto pTangent  = (pSrcMesh->HasTangentsAndBitangents()) ? &(pSrcMesh->mTangents[i])  : &zero3D;  /* 節ベクトル */


		// メッシュに格納する
		dstMesh.Vertices[i] = MeshVertex(
			DirectX::XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
			DirectX::XMFLOAT2(pTexCoord->x, pTexCoord->y),
			DirectX::XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
			DirectX::XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
		);
	}


	/* インデックス情報について */

	// 頂点インデックスのメモリを確保（一つのポリゴンに頂点は３つなので ×３を計算）
	std::cout << "NumFaces : " << pSrcMesh->mNumFaces << std::endl;
	dstMesh.Indices.resize(pSrcMesh->mNumFaces * 3);

	for (auto i = 0u; i < pSrcMesh->mNumFaces; i++) {

		const auto& face = pSrcMesh->mFaces[i];
		assert(face.mNumIndices == 3);           /* 頂点数は必ず３になる */

		dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
		dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
		dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
	}
}



void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial) {

	/* マテリアル情報を分解する */

	// 拡散反射成分の読み取り
	{

		// 読み込まれた情報を格納する変数
		aiColor3D color(0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {

			// 読み込みが成功したらマテリアル情報に格納
			dstMaterial.Diffuse.x = color.r;
			dstMaterial.Diffuse.y = color.g;
			dstMaterial.Diffuse.z = color.b;
		}
		else {

			// 失敗したら 0.5fで初期化
			dstMaterial.Diffuse.x = 0.5f;
			dstMaterial.Diffuse.y = 0.5f;
			dstMaterial.Diffuse.z = 0.5f;
		}
	}

	
	// 鏡面反射成分の読み取り
	{
		aiColor3D color(0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {

			// 読み取りに成功したらマテリアル情報に格納
			dstMaterial.Specular.x = color.r;
			dstMaterial.Specular.y = color.g;
			dstMaterial.Specular.z = color.b;
		}
		else {

			// 失敗したら 0.5fで初期化
			dstMaterial.Specular.x = 0.5f;
			dstMaterial.Specular.y = 0.5f;
			dstMaterial.Specular.z = 0.5f;
		}
	}


	// 鏡面反射強度の読み取り
	{
		float shininess = 0.0f;

		if (pSrcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {

			// 読み取りに成功したらマテリアル情報に格納
			dstMaterial.Shininess = shininess;
		}
		else {

			// 失敗したら 0.0fで初期化
			dstMaterial.Shininess = 0.0f;
		}
	}


	// ディフューズマップの読み取り
	{
		aiString path;

		if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS) {

			// 読み取りに成功したらマルチバイト文字列に変換後パスを格納
			dstMaterial.DiffuseMap = std::string(path.C_Str());
		}
		else {

			// 失敗したら文字列は空に
			dstMaterial.DiffuseMap.clear();
		}
	}


	// 法線マップの読み取り
}

}//


/****************************************************************
 * メッシュのロード
 ****************************************************************/
bool LoadMesh(
	const wchar_t*          filename,     /* ファイルまでのパス */
	std::vector<Mesh>&      meshes,       /* メッシュ情報の格納先アドレス */
	std::vector<Material>&  materials) {  /* マテリアル情報の格納先アドレス */


	// メッシュの読み込みを行い引数の変数に結果を格納する
	MeshLoader loader;
	return loader.Load(filename, meshes, materials);
}