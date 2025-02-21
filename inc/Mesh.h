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
 * MeshVertex �\���́i3D���f���̒��_���Ȃǂ��i�[�j
 ****************************************************************/
struct MeshVertex{

	DirectX::XMFLOAT3 Position;  /* ���_���W */
	DirectX::XMFLOAT2 TexCoord;  /* uv���W */
	DirectX::XMFLOAT3 Normal;    /* �@���x�N�g�� */
	DirectX::XMFLOAT3 Tangent;   /* �ڐ��x�N�g�� */

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


	// �V�F�[�_�[�ւ̓��͂̌`���i���̓t�H�[�}�b�g�z��Ƃ��̔z��̗v�f���j
	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementNum = 4;                                  /* ���͂̐� */
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementNum];  /* ���̓t�H�[�}�b�g */
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
	std::string       NormalMap;   /* �@���}�b�v�ւ̃p�X */

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
 * Mesh �N���X
 ****************************************************************/
class Mesh {

public:

	std::vector<MeshVertex> Vertices;    /* ���_��� */
	std::vector<uint32_t>   Indices;     /* ���_�C���f�b�N�X��� */
	uint32_t                MaterialId;  /* �}�e���A���ԍ� */
};



/****************************************************************
 * ���b�V���̓ǂݍ��݁i�ق��̃N���X�ł̎Q�ƂɎg�p�j
 ****************************************************************/
bool LoadMesh(
	const wchar_t* filename,             /* �ǂݍ��ރt�@�C���܂ł̃p�X */
	std::vector<Mesh>& meshes,           /* �ǂݍ��݌�̃��b�V�����̊i�[�� */
	std::vector<Material>& materials);   /* �ǂݍ��݌�̃}�e���A�����̊i�[�� */
