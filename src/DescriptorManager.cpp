
/****************************************************************
 * Include
 ****************************************************************/
#include "DescriptorManager.h"
#include <iostream>



DescriptorManager::DescriptorManager():
	m_pHeapRTV         (nullptr),
	m_pHeapDSV         (nullptr),
	m_pHeapCBV_SRV_UAV (nullptr),
	m_HandleRTV        (),
	m_HandleDSV        (),
	m_HeapCount        (0)
{ /* DO_NOTHING */ }


DescriptorManager::~DescriptorManager()
{ /* DO_NOTHING */ }



bool DescriptorManager::CreateRTV(ID3D12Device* pDevice, IDXGISwapChain3* pSwapChain) {

	if (pDevice == nullptr || pSwapChain == nullptr) {

		std::cout << "Error : DSV�����G���[" << std::endl;
		return false;
	}

	// �f�B�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;   /* RTV��ݒ� */
	desc.NumDescriptors = _FrameCount;                      /* �f�B�X�N���v�^�̐��i�t���[���̐����j*/
	desc.NodeMask       = 0;                                /* ������GPU�͂Ȃ� */
	desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* �V�F�[�_�[����A�N�Z�X�\ */


	// �f�B�X�N���v�^�q�[�v�̐���
	HRESULT hr = pDevice->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create RTV DescriptorHeap." << std::endl;
		return false;
	}


	// �f�B�X�N���v�^�q�[�v�̐擪�A�h���X���擾
	auto handleCPU = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();

	// GPU�ŗL�̃C���N�������g�T�C�Y�̎擾
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


	// �����_�[�^�[�Q�b�g�̐���
	for (auto i = 0u; i < _FrameCount; i++) {

		hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pRTB[i].GetAddressOf()));
		if (FAILED(hr)) {

			std::cout << "Error : Can't create RenderTargetBuffer." << std::endl;
			return false;
		}

		// �����_�[�^�[�Q�b�g�r���[�iRTV�j�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  /* �f�B�X�v���C�ւ̕\���t�H�[�}�b�g */
		desc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;    /* �r���[�̎��� */
		desc.Texture2D.MipSlice   = 0;                                /* �~�b�v���x���ݒ� */
		desc.Texture2D.PlaneSlice = 0;                                /* ���ʃX���C�X�ݒ� */

		// �����_�[�^�[�Q�b�g�r���[�iRTV�j�̐���
		pDevice->CreateRenderTargetView(
			m_pRTB[i].Get(),
			&desc,
			handleCPU);

		m_HandleRTV[i] = handleCPU;      // �����_�[�^�[�Q�b�g�r���[�̐擪�A�h���X���i�[
		handleCPU.ptr += incrementSize;  // �|�C���^���C���N�������g���������i����RTV�̃A�h���X�܂œ������j
	}

	// ����I��
	return true;
}


bool DescriptorManager::CreateDSV(ID3D12Device* pDevice, uint32_t Width, uint32_t Height) {

	if (pDevice == nullptr || Width == 0 || Height == 0) {

		std::cout << "Error : DSV�����G���[" << std::endl;
		return false;
	}

	// ���\�[�X�̃v���p�e�B�ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type                 = D3D12_HEAP_TYPE_DEFAULT;          /* GPU�A�N�Z�X�̓ǂݏ����̂݁ECPU�̓A�N�Z�X�s�� */
	prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�̃y�[�W�v���p�e�B */
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���ݒ� */
	prop.CreationNodeMask     = 1;                                /* GPU�̐� */
	prop.VisibleNodeMask      = 1;                                /* GPU���ʎ��ɐݒ� */

	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;       /* ���\�[�X�̎��� */
	desc.Alignment          = 0;                                        /* �o�b�t�@�̋�؂� */
	desc.Width              = Width;                                    /* ���\�[�X�̉��� */
	desc.Height             = Height;                                   /* ���\�[�X�̏c�� */
	desc.DepthOrArraySize   = 1;                                        /* ���\�[�X�̉��s */
	desc.Format             = DXGI_FORMAT_D32_FLOAT;                    /* �t�H�[�}�b�g */
	desc.MipLevels          = 1;                                        /* �~�b�v���x�� */
	desc.SampleDesc.Count   = 1;                                        /* �}���`�T���v�����O�i�A���`�G�C���A�X�j�ݒ� */
	desc.SampleDesc.Quality = 0;                                        /* �}���`�T���v�����O�̃N�I���e�B�ݒ� */
	desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;             /* ���C�A�E�g�ݒ� */
	desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;  /* �[�x�X�e���V�����w�� */


	// �[�x�X�e���V���o�b�t�@�̃N���A�l��ݒ�
	D3D12_CLEAR_VALUE clear;
	clear.Format               = DXGI_FORMAT_D32_FLOAT;  /* �[�x�X�e���V���o�b�t�@�̃t�H�[�}�b�g */
	clear.DepthStencil.Depth   = 1.0f;                   /* �[�x�l�̏����l */
	clear.DepthStencil.Stencil = 0;                      /* �X�e���V���l�̏����l */

	// �[�x�X�e���V���o�b�t�@�̐���
	HRESULT hr = pDevice->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clear,
		IID_PPV_ARGS(m_pDSB.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create Depth Stencil Buffer." << std::endl;
		return false;
	}


	// �f�B�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;                                /* �f�B�X�N���v�^�i�r���[�j�̐� */
	heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   /* �f�B�X�N���v�^�[�̎�ސݒ� */
	heapDesc.NodeMask       = 0;                                /* ����GPU������ꍇ�ݒ� */
	heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  /* �V�F�[�_�[����͌���Ȃ��悤�ɂ��� */

	hr = pDevice->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(m_pHeapDSV.GetAddressOf()));
	if (FAILED(hr)) {

		std::cout << "Error : Can't create DSV DescriptorHeap." << std::endl;
	}

	
	// DSV�p�f�B�X�N���v�^�[�q�[�v�̐擪�n���h�����擾
	auto handleCPU = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();

	// DSV�p�̃C���N�������g�T�C�Y���擾
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// �[�x�X�e���V���o�b�t�@�r���[�̐ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC DSVdesc = {};
	DSVdesc.Format             = DXGI_FORMAT_D32_FLOAT;          /* DSV�̃t�H�[�}�b�g */
	DSVdesc.Texture2D.MipSlice = 0;                              /* �~�b�v�X���C�X */
	DSVdesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;  /* �r���[�̎��� */
	DSVdesc.Flags              = D3D12_DSV_FLAG_NONE;            /* �ǂݎ��E�������ݐݒ�͂��Ȃ� */


	// DSV�̐���
	pDevice->CreateDepthStencilView(
		m_pDSB.Get(),
		&DSVdesc,
		handleCPU);


	// DSV�擪�A�h���X���i�[
	m_HandleDSV = handleCPU;

	// ����I��
	return true;
}


bool DescriptorManager::CreateCBV(ID3D12Device* pDevice, ID3D12DescriptorHeap* pHeap, ConstantBuffer* pCBV, size_t size) {

	if (pDevice == nullptr || pHeap == nullptr || pCBV == nullptr || size == 0) {

		std::cout << "CreateCBV Error : �����������Ȓl" << std::endl;
		return false;
	}

	// CBV�̐���
	if (!pCBV->Init(pDevice, pHeap, size, m_HeapCount)) {

		std::cout << "Create Error : CBV�̐����G���[" << std::endl;
		return false;
	}

	// �����J�E���g
	AddCount();

	
	return true;
}


bool DescriptorManager::CreateSRV(
	ID3D12Device*                 pDevice, 
	ID3D12DescriptorHeap*         pHeap, 
	Texture*                      pSRV, 
	const wchar_t*                filename,
	DirectX::ResourceUploadBatch& batch)
{

	if (pDevice == nullptr || pHeap == nullptr) {

		std::cout << "Error : ������ nullptr�G���[" << std::endl;
		return false;
	}


	// SRV�̍쐬
	if (!pSRV->Init(pDevice, pHeap, filename, batch, m_HeapCount)) {

		std::cout << "Create Error : SRV�̐����G���[" << std::endl;
		return false;
	}

	// �����J�E���g
	AddCount();


	return true;
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::GetCPUHandle_RTV(const uint32_t FrameIndex) const{

	if (m_pHeapRTV == nullptr) {

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	return m_HandleRTV[FrameIndex];
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::GetCPUHandle_DSV() const{

	if (m_pHeapDSV == nullptr) {

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	return m_HandleDSV;
}


size_t DescriptorManager::GetCount() const {

	return m_HeapCount;
}


void DescriptorManager::Term() {

	TermRTV();
	TermDSV();
	m_HeapCount = 0;
}


void DescriptorManager::TermRTV() {

	// �����_�[�^�[�Q�b�g�r���[�̔j��
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < _FrameCount; i++) {

		m_pRTB[i].Reset();
	}

}


void DescriptorManager::TermDSV() {

	// �[�x�X�e���V���r���[�̔j��
	m_pHeapDSV.Reset();
	m_pDSB.Reset();
}