
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
 * �Œ�l
 ****************************************************************/
#define FORMAT_FLOAT3   DXGI_FORMAT_R32G32B32_FLOAT                 /* float�^ 4�o�C�g��RGB�t�H�[�}�b�g */
#define FORMAT_FLOAT2   DXGI_FORMAT_R32G32_FLOAT                    /* float�^ 4�o�C�g��RG�t�H�[�}�b�g */
#define APPEND_ALIGNED  D3D12_APPEND_ALIGNED_ELEMENT                /* ���̓��C�A�E�g���A�����Ă��邱�Ƃ�\�� */
#define IN_VERTEX_DATA  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA  /* ���͗v�f�𒸓_���ƃf�[�^�Ƃ��Ĉ������Ƃ�\�� */


/****************************************************************
 * ���_���̓��̓��C�A�E�g
 ****************************************************************/
const D3D12_INPUT_ELEMENT_DESC MeshVertex::InputElements[] = {

	{"POSITION", 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"TEXCOORD", 0, FORMAT_FLOAT2, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"NORMAL"  , 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0},
	{"TANGENT" , 0, FORMAT_FLOAT3, 0, APPEND_ALIGNED, IN_VERTEX_DATA, 0}
};

// ���͗v�f�Ɠ��͂̐���\��
const D3D12_INPUT_LAYOUT_DESC MeshVertex::InputLayout = {

	MeshVertex::InputElements,   /* ���̓t�H�[�}�b�g�z�� */
	MeshVertex::InputElementNum  /* �z��̗v�f�� */
};



/****************************************************************
 * wchar_t�^ ���� char�^�iutf-8�j�ɕϊ�����
 ****************************************************************/
std::string ToUTF8(const std::wstring& str) {

	// ���C�h�����̒������}���`�o�C�g�̒����ɕϊ�
	auto length = WideCharToMultiByte(CP_UTF8, 0u, str.data(), -1, nullptr, 0, nullptr, nullptr);


	// �}���`�o�C�g���̒�����char�𓮓I�Ɏ擾
	auto buffer = new char[length];


	// ���C�h�������}���`�o�C�g�ɕϊ�
	WideCharToMultiByte(CP_UTF8, 0u, str.data(), -1, buffer, length, nullptr, nullptr);

	// ���ʂ��i�[
	std::string result(buffer);

	// ���I�擾�����o�b�t�@�̊J��
	delete[] buffer;
	buffer = nullptr;

	return result;
}



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
	if (filename == nullptr) {

		std::cout << "�t�@�C���p�X������܂���" << std::endl;
		return false;
	}

	// wchar_t�^ ���� char�^�iUTF-8�j�ɕϊ�
	auto filePath = ToUTF8(filename);


	// �ǂݍ��ݎ��̃I�v�V�����ݒ�
	Assimp::Importer importer;
	int ImportFlag = 0;
	ImportFlag |= aiProcess_Triangulate;               /* �S�Ẵ��b�V���̑S�Ă̖ʃf�[�^���O�p�`������ */
	ImportFlag |= aiProcess_PreTransformVertices;      /* �m�[�h�O���t���폜���A����炪�����[�J���ϊ��s��őS�Ă̒��_�����O�ɕϊ� */
	ImportFlag |= aiProcess_CalcTangentSpace;          /* �ǂݍ��܂ꂽ���b�V���̐ڐ��x�N�g���Ə]�ڐ�x�N�g�����v�Z */
	ImportFlag |= aiProcess_GenSmoothNormals;          /* ���b�V���̂��ׂĂ̒��_�ɑ΂��Ė@���׃N�g���𐶐� */
	ImportFlag |= aiProcess_GenUVCoords;               /* ��UV�}�b�s���O��K�؂ȍ��W�`�����l���ɕϊ� */
	ImportFlag |= aiProcess_RemoveRedundantMaterials;  /* �璷�܂��͎Q�Ƃ���Ă��Ȃ��}�e���A�����������폜���� */
	ImportFlag |= aiProcess_OptimizeMeshes;            /* ���b�V�������œK������ */

	// �t�@�C����ǂݍ���
	auto pScene = importer.ReadFile(filePath, ImportFlag);


	// �ǂݍ��ݎ��s�����m�F
	if (pScene == nullptr) {
		std::cout << "���f���t�@�C�����ǂݍ��߂܂���" << std::endl;
		return false;
	}


	// ���b�V���̃����������O�Ɋm��
	meshes.clear();
	meshes.resize(pScene->mNumMeshes);  // ���b�V���̐������m��
	std::cout << "NumMesh:" << meshes.size() << std::endl;

	// ���b�V���f�[�^��ϊ�����
	for (auto i = 0u; i < meshes.size(); i++) {

		const auto pMesh = pScene->mMeshes[i];  // ���b�V����ǂݍ���
		ParseMesh(meshes[i], pMesh);            // ���b�V���𕪊�
	}


	// �}�e���A���̃����������O�Ɋm��
	materials.clear();
	materials.resize(pScene->mNumMaterials);  // ���b�V���̐������m��

	// �}�e���A���f�[�^��ϊ�����
	for (auto i = 0u; i < materials.size(); i++) {

		auto pMaterial = pScene->mMaterials[i];  // �}�e���A������ǂݍ���
		ParseMaterial(materials[i], pMaterial);  // �}�e���A���𕪊�
	}

	// �V�[����j��
	pScene = nullptr;

	std::cout << "���f���ǂݍ��݊���" << std::endl;
	// ����I��
	return true;
}



void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh) {

	/* ���_���ɂ��� */
	
	// �}�e���A���ԍ���ݒ�
	dstMesh.MaterialId = pSrcMesh->mMaterialIndex;

	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	// ���_���������O�m��
	dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
	std::cout << "NumVertices : " << pSrcMesh->mNumVertices << std::endl;

	for (auto i = 0u; i < pSrcMesh->mNumVertices; i++) {

		auto pPosition = &(pSrcMesh->mVertices[i]);                 /* ���W */
		auto pNormal   = &(pSrcMesh->mNormals[i]);                  /* �@���x�N�g�� */
		auto pTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;  /* uv���W */
		auto pTangent  = (pSrcMesh->HasTangentsAndBitangents()) ? &(pSrcMesh->mTangents[i])  : &zero3D;  /* �߃x�N�g�� */


		// ���b�V���Ɋi�[����
		dstMesh.Vertices[i] = MeshVertex(
			DirectX::XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
			DirectX::XMFLOAT2(pTexCoord->x, pTexCoord->y),
			DirectX::XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
			DirectX::XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
		);
	}


	/* �C���f�b�N�X���ɂ��� */

	// ���_�C���f�b�N�X�̃��������m�ہi��̃|���S���ɒ��_�͂R�Ȃ̂� �~�R���v�Z�j
	std::cout << "NumFaces : " << pSrcMesh->mNumFaces << std::endl;
	dstMesh.Indices.resize(pSrcMesh->mNumFaces * 3);

	for (auto i = 0u; i < pSrcMesh->mNumFaces; i++) {

		const auto& face = pSrcMesh->mFaces[i];
		assert(face.mNumIndices == 3);           /* ���_���͕K���R�ɂȂ� */

		dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
		dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
		dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
	}
}



void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial) {

	/* �}�e���A�����𕪉����� */

	// �g�U���ː����̓ǂݎ��
	{

		// �ǂݍ��܂ꂽ�����i�[����ϐ�
		aiColor3D color(0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {

			// �ǂݍ��݂�����������}�e���A�����Ɋi�[
			dstMaterial.Diffuse.x = color.r;
			dstMaterial.Diffuse.y = color.g;
			dstMaterial.Diffuse.z = color.b;
		}
		else {

			// ���s������ 0.5f�ŏ�����
			dstMaterial.Diffuse.x = 0.5f;
			dstMaterial.Diffuse.y = 0.5f;
			dstMaterial.Diffuse.z = 0.5f;
		}
	}

	
	// ���ʔ��ː����̓ǂݎ��
	{
		aiColor3D color(0.0f, 0.0f, 0.0f);

		if (pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {

			// �ǂݎ��ɐ���������}�e���A�����Ɋi�[
			dstMaterial.Specular.x = color.r;
			dstMaterial.Specular.y = color.g;
			dstMaterial.Specular.z = color.b;
		}
		else {

			// ���s������ 0.5f�ŏ�����
			dstMaterial.Specular.x = 0.5f;
			dstMaterial.Specular.y = 0.5f;
			dstMaterial.Specular.z = 0.5f;
		}
	}


	// ���ʔ��ˋ��x�̓ǂݎ��
	{
		float shininess = 0.0f;

		if (pSrcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {

			// �ǂݎ��ɐ���������}�e���A�����Ɋi�[
			dstMaterial.Shininess = shininess;
		}
		else {

			// ���s������ 0.0f�ŏ�����
			dstMaterial.Shininess = 0.0f;
		}
	}


	// �f�B�t���[�Y�}�b�v�̓ǂݎ��
	{
		aiString path;

		if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS) {

			// �ǂݎ��ɐ���������}���`�o�C�g������ɕϊ���p�X���i�[
			dstMaterial.DiffuseMap = std::string(path.C_Str());
		}
		else {

			// ���s�����當����͋��
			dstMaterial.DiffuseMap.clear();
		}
	}


	// �@���}�b�v�̓ǂݎ��
}

}//


/****************************************************************
 * ���b�V���̃��[�h
 ****************************************************************/
bool LoadMesh(
	const wchar_t*          filename,     /* �t�@�C���܂ł̃p�X */
	std::vector<Mesh>&      meshes,       /* ���b�V�����̊i�[��A�h���X */
	std::vector<Material>&  materials) {  /* �}�e���A�����̊i�[��A�h���X */


	// ���b�V���̓ǂݍ��݂��s�������̕ϐ��Ɍ��ʂ��i�[����
	MeshLoader loader;
	return loader.Load(filename, meshes, materials);
}