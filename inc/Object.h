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
 * Object �N���X
 ****************************************************************/
class Object {

private:

	DirectX::XMFLOAT3          Pos;             /* ���W */
	Mesh                       m_mesh;          /* ���b�V����� */
	D3D12_VERTEX_BUFFER_VIEW   m_VBV;           /* ���_�o�b�t�@�r���[ */
	D3D12_INDEX_BUFFER_VIEW    m_IBV;           /* �C���f�b�N�X�o�b�t�@�r���[ */
	ConstantBuffer             m_Transform[2];  /* �ϊ��s�� */
	ConstantBuffer             m_Light;         /* ���C�g */
	ConstantBuffer             m_Material;      /* �}�e���A�� */
	Texture                    m_DiffuseMap;    /* �f�B�t���[�Y�}�b�v */
	Texture                    m_NormalMap;     /* �@���}�b�v */

public:

	Object(DirectX::XMFLOAT3 Position);

	~Object();

	
	/****************************************************************
	 * ����������
	 ****************************************************************/
	virtual void Init(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap);


	/****************************************************************
	 * �j������
	 ****************************************************************/
	virtual void Term();


	/****************************************************************
	 * �X�V����
	 ****************************************************************/
	virtual void Update();


	/****************************************************************
	 * �`�揈��
	 ****************************************************************/
	virtual void Render(ID3D12GraphicsCommandList* pCmdList);
};