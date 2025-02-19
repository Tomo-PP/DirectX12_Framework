
/****************************************************************
 * Include
 ****************************************************************/
#include "ConstantBuffer.h"
#include <iostream>

ConstantBuffer::ConstantBuffer() :
	m_pCB       (nullptr),
	m_HandleCPU (),
	m_HandleGPU (),
	m_Desc      (),
	m_pMapBuf   (nullptr)
{ /* DO_NOTHING */ }


ConstantBuffer::~ConstantBuffer()
{ /* DO_NOTHING */ }


bool ConstantBuffer::Init(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap, size_t size, size_t HeapCount) {

	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;    // �A���C�����g�T�C�Y�i256 Byte�j
	uint64_t alignmentSize = (size + (align - 1)) & ~(align - 1);   // �A���C�����g�T�C�Y�̌v�Z

	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�iCPU�̏������ݕ��@�ɂ��āE����͎w��Ȃ��j*/
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */

	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j */
	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	resourceDesc.Width              = alignmentSize;                    /* �f�[�^�T�C�Y�F256 Byte�i256 Byte�𒴂���ꍇ 512 Byte�j*/
	resourceDesc.Height             = 1;                                /* �f�[�^�̏c�̃T�C�Y�i�o�b�t�@�Ȃ̂łP�j*/
	resourceDesc.DepthOrArraySize   = 1;                                /* �f�[�^�̉��s�i�o�b�t�@�Ȃ̂łP�j*/
	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ�i����͎g��Ȃ��̂łO�j */
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �n�܂肩��I���܂ŘA�������o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j */


	// �f�B�X�N���v�^�q�[�v���̃f�[�^�ɂ��āA���̃f�[�^�Ɉړ����邽�߂̃C���N�������g�T�C�Y���擾
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	HRESULT hr = pDevice->CreateCommittedResource(
		&heapProp,                               /* �q�[�v�̐ݒ� */
		D3D12_HEAP_FLAG_NONE,                    /* �q�[�v�̃I�v�V���� */
		&resourceDesc,                           /* ���\�[�X�̐ݒ� */
		D3D12_RESOURCE_STATE_GENERIC_READ,       /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
		nullptr,                                 /* RTV��DSV�p�̐ݒ� */
		IID_PPV_ARGS(m_pCB.GetAddressOf()));     /* �A�h���X���i�[ */
	if (FAILED(hr)) {

		return false;
	}

	// �萔�o�b�t�@�̃A�h���X
	auto addressGPU = m_pCB->GetGPUVirtualAddress();               // GPU�̉��z�A�h���X���擾
	auto handleCPU = pHeap->GetCPUDescriptorHandleForHeapStart();  // �f�B�X�N���v�^�q�[�v�̐擪�n���h�����擾�iCPU�j
	auto handleGPU = pHeap->GetGPUDescriptorHandleForHeapStart();  // �f�B�X�N���v�^�q�[�v�̐擪�n���h�����擾�iGPU�j

	// �萔�o�b�t�@�̐擪�|�C���^���v�Z�i�萔�o�b�t�@�̃T�C�Y�����C���N�������g�j
	handleCPU.ptr += incrementSize * HeapCount;
	handleGPU.ptr += incrementSize * HeapCount;

	// �萔�o�b�t�@�r���[�̐ݒ�i�萔�o�b�t�@�̏���ۑ��j
	m_HandleCPU           = handleCPU;          // �萔�o�b�t�@�̐擪�n���h���iCPU�j
	m_HandleGPU           = handleGPU;          // �萔�o�b�t�@�̐擪�n���h���iGPU�j
	m_Desc.BufferLocation = addressGPU;         // �o�b�t�@�̕ۑ��ʒu���w��
	m_Desc.SizeInBytes    = alignmentSize;      // �萔�o�b�t�@�̃T�C�Y

	// �萔�o�b�t�@�r���[�̐���
	pDevice->CreateConstantBufferView(&m_Desc, handleCPU);


	//�萔�o�b�t�@�iTransform�j�̃}�b�s���O���s��
	hr = m_pCB->Map(0, nullptr, reinterpret_cast<void**>(&m_pMapBuf));  // ��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	if (FAILED(hr)) {

		std::cout << "�萔�o�b�t�@�̃}�b�v�G���[" << std::endl;
		return false;
	}

	return true;
}


void ConstantBuffer::Term() {

	// �萔�o�b�t�@�̔j��
	if (m_pCB != nullptr) {

		m_pCB->Unmap(0, nullptr);

		m_HandleCPU.ptr       = 0;
		m_HandleGPU.ptr       = 0;
		m_Desc.SizeInBytes    = 0;
		m_Desc.BufferLocation = 0;
		m_pMapBuf             = 0;
	}
	
	m_pCB.Reset();
}


D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleCPU() const {

	return m_HandleCPU;
}


D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleGPU() const {

	return m_HandleGPU;
}


D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetVirtualAddress() const {

	return m_Desc.BufferLocation;
}