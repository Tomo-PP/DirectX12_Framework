
/****************************************************************
 * Include
 ****************************************************************/
#include "Texture.h"
#include "FileUtil.h"
#include <iostream>


Texture::Texture():
	m_pResource  (nullptr),
	m_HandleCPU  (),
	m_HandleGPU  ()
{ /* DO_NOTHING */ }


Texture::~Texture()
{ /* DO_NOTHING */ }


bool Texture::Init(
	ID3D12Device*                 pDevice, 
	ID3D12DescriptorHeap*         pHeap, 
	const wchar_t*                filename, 
	DirectX::ResourceUploadBatch& batch,
	size_t                        HeapCount)
{

	if (pDevice == nullptr || pHeap == nullptr || filename == nullptr) {

		std::cout << "Texture Error : ������nullptr" << std::endl;
		return false;
	}

	// �t�@�C���p�X�̌���
	std::wstring texturePath;
	if (!SearchFilePath(filename, texturePath)) {

		std::cout << "Texture Error : �e�N�X�`����������Ȃ�����" << std::endl;
		return false;
	}

	// ���\�[�X�̐����i���̊֐��ň�C�Ƀ��\�[�X�𐶐��ł���B���\�[�X�ݒ肩�炵�Ȃ��Ă��悢�j
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		pDevice,                             /* �f�o�C�X��n�� */
		batch,                               /* batch */
		texturePath.c_str(),                 /* �e�N�X�`���܂ł̃p�X�i���C�h�������}���`�o�C�g�ɕϊ����ēn���K�v����j*/
		m_pResource.GetAddressOf(),          /* �e�N�X�`���̃A�h���X���擾 */
		true);                               /*  */
	if (FAILED(hr)) {

		std::cout << "Texture Error : �e�N�X�`�����\�[�X�̐����G���[" << std::endl;
		return false;
	}

	// �C���N�������g�T�C�Y���擾�i�V�F�[�_�[�p�iSRV�j�̃������̃C���N�������g�T�C�Y�j
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// CPU�f�B�X�N���v�^�n���h����GPU�f�B�X�N���v�^�n���h�����f�B�X�N���v�^�q�[�v����擾
	auto handleCPU = pHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleGPU = pHeap->GetGPUDescriptorHandleForHeapStart();

	// �e�N�X�`���Ƀf�B�X�N���v�^�n���h�������蓖�Ă�iCBV�~4���΂��K�v����j
	handleCPU.ptr += incrementSize * HeapCount;
	handleGPU.ptr += incrementSize * HeapCount;

	m_HandleCPU = handleCPU;
	m_HandleGPU = handleGPU;

	// �e�N�X�`���̍\�����擾�i��̃��C�u�����Ő������ꂽ�\����ǂݍ��ށj
	auto textureDesc = m_pResource->GetDesc();

	// �V�F�[�_���\�[�X�r���[�̐ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	// �V�F�[�_�[���\�[�X�r���[�̐���
	pDevice->CreateShaderResourceView(
		m_pResource.Get(),          /* ���\�[�X */
		&SRVDesc,                   /* SRV�̐ݒ� */
		m_HandleCPU);               /* CPU�f�B�X�N���v�^�n���h�� */

	return true;
}


void Texture::Term() {

	// ���\�[�X�̔j��
	m_pResource.Reset();
	m_HandleCPU.ptr = 0;
	m_HandleGPU.ptr = 0;

}