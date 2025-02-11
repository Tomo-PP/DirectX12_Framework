
/****************************************************************
 * Includes
 ****************************************************************/
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <codecvt>
#include <cassert>


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
}



void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh) {

	/* メッシュ情報を分解する */
}



void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial) {

	/* マテリアル情報を分解する */
}

}//