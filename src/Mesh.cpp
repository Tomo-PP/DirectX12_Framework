
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
 * ���b�V���̓ǂݍ��݁E��͂��s���N���X
 ****************************************************************/
namespace {

class MeshLoader {

public:

	MeshLoader();

	~MeshLoader();



	/****************************************************************
	 * ���b�V���̓ǂݍ��݁i���߂ēǂݍ��ޏꍇ�j
	 ****************************************************************/
	bool Load(
		const wchar_t*         filename,    /* �ǂݍ��݂����t�@�C���̃p�X�i���C�h�����̐擪�|�C���^�j */
		std::vector<Mesh>&     meshes,      /* ���b�V����� */
		std::vector<Material>& materials);  /* �}�e���A����� */


private:

	/****************************************************************
	 * ���b�V���f�[�^�̉��
	 ****************************************************************/
	void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);



	/****************************************************************
	 * �}�e���A�����̉��
	 ****************************************************************/
	void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);

};

	
MeshLoader::MeshLoader()
{ /* DO_NOTHING */ }

MeshLoader::~MeshLoader()
{ /* DO_NOTHING */ }



bool MeshLoader::Load(const wchar_t* filename, std::vector<Mesh>& meshes, std::vector<Material>& materials) {

	/* ���f���̃��[�h���� */
}



void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh) {

	/* ���b�V�����𕪉����� */
}



void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial) {

	/* �}�e���A�����𕪉����� */
}

}//