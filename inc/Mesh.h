#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>



/****************************************************************
 * MeshVertex �\���́i3D���f���̒��_���Ȃǂ��i�[�j
 ****************************************************************/
struct MeshVertex{

	DirectX::XMFLOAT3 Position;  /* ���_���W */
	DirectX::XMFLOAT2 TexCoord;  /* uv���W */
	DirectX::XMFLOAT3 Normal;    /* �@���x�N�g�� */
	DirectX::XMFLOAT3 Tangent;   /* �ڃx�N�g�� */
};



/****************************************************************
 * Material �\���́i3D���f���̃}�e���A�������i�[�j
 ****************************************************************/
struct Material {

	DirectX::XMFLOAT3 Diffuse;     /* �g�U���˗� */
	DirectX::XMFLOAT3 Specular;    /* ���ʔ��˗� */
	float             alpha;       /* ���ߐ��� */
	float             Shininess;   /* ���ʔ��ˋ��x */
	std::string       DiffuseMap;  /* �e�N�X�`���ւ̃p�X */
};



/****************************************************************
 * Mesh �N���X
 ****************************************************************/
class Mesh {

private:

	std::vector<MeshVertex> vertices;    /* ���_��� */
	std::vector<uint32_t>   indices;     /* ���_�C���f�b�N�X��� */
	uint32_t                materialId;  /* �}�e���A���ԍ� */

public:
	
	/****************************************************************
	 * ���b�V���̓ǂݍ��݁i�ق��̃N���X�ł̎Q�ƂɎg�p�j
	 ****************************************************************/
	bool LoadMesh(
		const wchar_t*          filename,   /* �ǂݍ��ރt�@�C���܂ł̃p�X */
		std::vector<Mesh>&      mesh,       /* �ǂݍ��݌�̃��b�V�����̊i�[�� */
		std::vector<Material>&  material);  /* �ǂݍ��݌�̃}�e���A�����̊i�[�� */
};

