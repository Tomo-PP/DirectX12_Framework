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
 * �N���X
 ****************************************************************/
class VerteBuffer;
class IndexBuffer;
class ConstantBuffer;
class Texture;
class DescriptorManaer;


/****************************************************************
 * Object �N���X
 ****************************************************************/
class Object {

private:

	DirectX::XMFLOAT3          Pos;             /* ���W */
	VertexBuffer               m_VB;            /* ���_�o�b�t�@ */
	IndexBuffer                m_IB;            /* �C���f�b�N�X�o�b�t�@ */

	ConstantBuffer             m_Transform[2];  /* �ϊ��s�� */
	ConstantBuffer             m_Light;         /* ���C�g */
	ConstantBuffer             m_Material;      /* �}�e���A�� */
	Texture                    m_DiffuseMap;    /* �f�B�t���[�Y�}�b�v */
	Texture                    m_NormalMap;     /* �@���}�b�v */

public:

	Object();

	Object(DirectX::XMFLOAT3 Position);

	~Object();

	
	/****************************************************************
	 * ����������
	 ****************************************************************/
	virtual bool Init(
		ID3D12Device*       pDevice,
		ID3D12CommandQueue* pCmdQueue,
		DescriptorManager*  DespManager,
		std::wstring        filename);


	/****************************************************************
	 * �j������
	 ****************************************************************/
	virtual void Term();


	/****************************************************************
	 * �X�V����
	 ****************************************************************/
	virtual void Update(uint32_t FrameIndex);


	/****************************************************************
	 * �`�揈��
	 ****************************************************************/
	virtual void Render(ID3D12GraphicsCommandList* pCmdList, uint32_t FrameIndex);


	/****************************************************************
	 * �ϊ��s��֘A�̊֐�
	 ****************************************************************/
	void ModelScaling(Vector3 Scale);      /* �X�P�[���ύX */

	void ModelRotation(float angle);       /* ��] */

	void ModelTranslation(Vector3 trans);  /* ���s�ړ� */

};