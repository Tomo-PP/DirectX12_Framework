
/****************************************************************
 * Include
 ****************************************************************/
#include "IndexBuffer.h"
#include <iostream>


IndexBuffer::IndexBuffer() :
	m_pIB        (nullptr),
	m_IBV        (),
	m_IndexCount (0)
{ /* DO_NOTHING */ }


IndexBuffer::~IndexBuffer()
{ /* DO_NOTHING */ }


bool IndexBuffer::Init(ID3D12Device* pDevice, const Mesh* mesh) {

	// �q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /*  */
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /*  */
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̐ݒ� */
	heapProp.CreationNodeMask     = 1;                                /* GPU�̎��ʃ}�X�N */
	heapProp.VisibleNodeMask      = 1;                                /*  */


	// �C���f�b�N�X�o�b�t�@�̃T�C�Y������
	m_IndexCount = mesh->Indices.size();
	auto IndexSize = sizeof(uint32_t) * m_IndexCount;  /* �C���f�b�N�X��uint32_t�^ �~ �C���f�b�N�X�̐� */
	auto indices   = mesh->Indices.data();             /* �}�b�s���O�p�C���f�b�N�X�f�[�^�̐擪�|�C���^���擾 */


	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j*/
	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	resourceDesc.Width              = IndexSize;                        /* �C���f�b�N�X��񂪓���T�C�Y�̃o�b�t�@�T�C�Y�i�e�N�X�`���̏ꍇ�͉������w��j*/
	resourceDesc.Height             = 1;                                /* �o�b�t�@�̏ꍇ�͂P�i�e�N�X�`���̏ꍇ�͏c�����w��j*/
	resourceDesc.DepthOrArraySize   = 1;                                /* ���\�[�X�̉��s�i�o�b�t�@�E�e�N�X�`���͂P�A�O�����e�N�X�`���͉��s�j*/
	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i����̓o�b�t�@�Ȃ̂� *UNKNOWN�B�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ� */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j*/


	// ���\�[�X�̐���
	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pIB.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "�C���f�b�N�X�o�b�t�@�̃G���[" << std::endl;
		return false;
	}


	// �}�b�s���O���s��
	void* ptr = nullptr;
	hr = m_pIB->Map(0, nullptr, &ptr);
	if (FAILED(hr)) {

		return false;
	}

	// �C���f�b�N�X�f�[�^��GPU�������ɃR�s�[����
	memcpy(ptr, indices, IndexSize);


	// �}�b�s���O�̉���
	m_pIB->Unmap(0, nullptr);


	// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
	m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();    /* �C���f�b�N�X�o�b�t�@��GPU������ */
	m_IBV.Format         = DXGI_FORMAT_R32_UINT;             /* �t�H�[�}�b�g�i�|���S�����������ꍇ *R32_UINT �|���S���������Ȃ��ꍇ *R16_UINT�j */
	m_IBV.SizeInBytes    = static_cast<UINT>(IndexSize);     /* �C���f�b�N�X�f�[�^�̃f�[�^�T�C�Y�i�o�C�g�j*/

	// ����I��
	return true;
}


void IndexBuffer::Term() {

	// �C���f�b�N�X�o�b�t�@�̔j��
	m_pIB.Reset();

	m_IBV.BufferLocation = 0;
	m_IBV.Format         = DXGI_FORMAT_UNKNOWN;
	m_IBV.SizeInBytes    = 0;
	m_IndexCount         = 0;
}


D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetIBV() {

	return m_IBV;
}

uint32_t IndexBuffer::GetIndexNum() const {

	return m_IndexCount;
}