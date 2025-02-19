
/****************************************************************
 * Include
 ****************************************************************/
#include "VertexBuffer.h"
#include "Mesh.h"
#include <iostream>


VertexBuffer::VertexBuffer() : 
	m_pVB  (nullptr),
	m_VBV  ()
{ /* DO_NOTHING */ }


VertexBuffer::~VertexBuffer()
{ /* DO_NOTHING */ }


bool VertexBuffer::Init(ID3D12Device* pDevice, size_t size, const Mesh* m_mesh) {

	if (pDevice == nullptr) {

		std::cout << "VertexBuffer Erro : ������nullptr�G���[" << std::endl;
		return false;
	}

	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�i����͎w��Ȃ��j*/
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */


	// �o�b�t�@�T�C�Y������
	auto VertexSize = sizeof(MeshVertex) * m_mesh[0].Vertices.size();  /* MeshVertex�i���_���j�~ ���_���̐� */
	auto vertices   = m_mesh[0].Vertices.data();                       /* �}�b�s���O�p�̒��_�f�[�^���m�ۂ���i�ϔz��̐擪�|�C���^���擾�j*/

	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j*/
	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	resourceDesc.Width              = VertexSize;                       /* ���_��񂪓���T�C�Y�̃o�b�t�@�T�C�Y�i�e�N�X�`���̏ꍇ�͉������w��j*/
	resourceDesc.Height             = 1;                                /* �o�b�t�@�̏ꍇ�͂P�i�e�N�X�`���̏ꍇ�͏c�����w��j*/
	resourceDesc.DepthOrArraySize   = 1;                                /* ���\�[�X�̉��s�i�o�b�t�@�E�e�N�X�`���͂P�A�O�����e�N�X�`���͉��s�j*/
	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ� */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j*/

	// ���_�o�b�t�@�iGPU���\�[�X�j�𐶐�
	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,                                   /* �q�[�v�̐ݒ� */
		D3D12_HEAP_FLAG_NONE,                        /* �q�[�v�̃I�v�V���� */
		&resourceDesc,                               /* ���\�[�X�̐ݒ� */
		D3D12_RESOURCE_STATE_GENERIC_READ,           /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
		nullptr,                                     /* RTV��DSV�p�̐ݒ� */
		IID_PPV_ARGS(m_pVB.GetAddressOf()));         /* �A�h���X���i�[ */
	if (FAILED(hr)) {

		std::cout << "VertexBuffer Error : ���_�o�b�t�@�̐����G���[" << std::endl;
		return false;
	}


	// ���_�o�b�t�@�̃}�b�s���O���s��
	void* ptr = nullptr;
	hr = m_pVB->Map(0, nullptr, &ptr);  //�@��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	if (FAILED(hr)) {

		return false;
	}


	// ���_�f�[�^���}�b�s���O��ɐݒ�iGPU�̃��������R�s�[�j
	memcpy(ptr, vertices, VertexSize);

	// �}�b�s���O�̉���
	m_pVB->Unmap(0, nullptr);


	// ���_�o�b�t�@�r���[�̐ݒ�i���_�o�b�t�@�̕`��R�}���h�p�EGPU�̃A�h���X��T�C�Y�Ȃǂ��L�����Ă����j
	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();                          /* ��قǃ}�b�v����GPU�̉��z�A�h���X���L�� */
	m_VBV.SizeInBytes    = static_cast<UINT>(VertexSize);                          /* ���_�f�[�^�S�̂̃T�C�Y���L�� */
	m_VBV.StrideInBytes  = static_cast<UINT>(sizeof(MeshVertex));                  /* �P���_�ӂ�̃T�C�Y���L�� */
}


void VertexBuffer::Term() {

	// ���_�o�b�t�@�̔j��
	m_pVB.Reset();

	m_VBV.BufferLocation = 0;
	m_VBV.SizeInBytes    = 0;
	m_VBV.StrideInBytes  = 0;

}


D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVBV() const {

	return m_VBV;
}