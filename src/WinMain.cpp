
/****************************************************************
 * Include
 ****************************************************************/
#include "WinMain.h"
#include <iostream>
#include <string>
#include <SimpleMath.h>


#if defined (DEBUG) || defined (_DEBUG)
	ComPtr<ID3D12DebugDevice> debugDevice;
#endif


App::App(uint32_t width, uint32_t height, LPCWSTR title):
	m_hInst       (nullptr),
	m_hWnd        (nullptr),
	m_Width       (width),
	m_Height      (height),
	m_windowTitle (title),

	m_pDevice     (nullptr),
	m_pCmdQueue   (nullptr),
	m_pSwapChain  (nullptr),
	m_pCmdList    (nullptr),
	m_pFence      (nullptr),

	//m_pVB         (nullptr),
	m_pPSO        (nullptr),
	m_FrameIndex  (0),
	//m_CBV         (),
	m_Scissor     (),
	m_Viewport    ()
{
	for (auto i = 0u; i < FrameCount; i++) {

		m_pCmdAllocator[i] = nullptr;
		m_FenceCounter[i]  = 0;
	}
}

App::~App()
{ }


void App::Run() {

	// ���C�����[�v
	if (InitApp()) {
		MainLoop();
	}

	// �A�v���̔j��
	TermApp();
	
}


bool App::InitApp() {

	// �E�B���h�E�̏�����
	if (!InitWindow()) {

		return false;
	}

	// Direct3D�̏�����
	if (!InitDirect3D()) {

		return false;
	}

	// �`��֘A�̏�����
	if (!OnInit()) {

		return false;
	}


	//����I��
	return true;
}


bool App::InitWindow() {

	// �C���X�^���X�n���h���̎擾�i���̃v���O�������̂̃C���X�^���X�j
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		std::cout << "Failed to Get Instance Handle." << std::endl;
		return false;
	}

	// �E�B���h�E�̐ݒ�
	WNDCLASSEX windowClass = {};
	windowClass.cbSize           = sizeof(WNDCLASSEX);                  /* �\���̂̃T�C�Y */
	windowClass.style            = CS_HREDRAW | CS_VREDRAW;             /* �E�B���h�E�N���X�̃X�^�C���i�E�B���h�E�̘g���Ȃǂ̐ݒ�j*/
	windowClass.lpfnWndProc      = WndProc;                             /* �E�B���h�E�v���V�[�W�� */
	windowClass.hIcon            = LoadIcon(hInst, IDI_APPLICATION);    /* �E�B���h�E�̃A�C�R���̃n���h���ݒ� */
	windowClass.hCursor          = LoadCursor(hInst, IDC_ARROW);        /* �E�B���h�E�̃}�E�X�J�[�\���ݒ� */
	windowClass.hbrBackground    = GetSysColorBrush(COLOR_BACKGROUND);  /* �E�B���h�E�̔w�i��`�悷��u���V�ւ̃n���h�� */
	windowClass.lpszMenuName     = nullptr;                             /* ���j���[�o�[�\���̍ۂɐݒ�i����͎g�p���Ȃ��̂� nullptr �j*/
	windowClass.lpszClassName    = m_windowTitle;                       /* �E�B���h�E�̃^�C�g�� */
	windowClass.hIconSm          = LoadIcon(hInst, IDI_APPLICATION);    /* �E�B���h�E�̃A�C�R���̐ݒ� */

	// �E�B���h�E�̓o�^
	if (!RegisterClassEx(&windowClass)) {
		return false;
	}


	// �C���X�^���X�n���h����ݒ�i�|�C���^�̎󂯓n���j
	m_hInst = hInst;


	// �E�B���h�E�T�C�Y�̐ݒ�i���Əオ���ꂼ��O�̊�j
	RECT rect = {};
	rect.right  = m_Width;   /* ���� */
	rect.bottom = m_Height;  /* ���� */


	// �E�B���h�E�T�C�Y�𒲐�
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rect, style, FALSE);


	// �E�B���h�E�𐶐�
	m_hWnd = CreateWindowEx(
		0,                       /* �g���E�B���h�E�̐ݒ�i����͎g�p���Ȃ��j*/
		m_windowTitle,           /* �E�B���h�E�N���X���̐ݒ� */
		m_windowTitle,           /* �E�B���h�E�̃^�C�g���o�[�ɕ\������閼�O�̐ݒ� */
		style,                   /* �E�B���h�E�̃X�^�C���ݒ�i���j���[�o�[�Ȃǂ̐ݒ�j*/
		CW_USEDEFAULT,           /* �E�B���h�E�̏��� x���W */
		CW_USEDEFAULT,           /* �E�B���h�E�̏��� y���W */
		rect.right - rect.left,  /* �E�B���h�E�� */
		rect.bottom - rect.top,  /* �E�B���h�E�̍��� */
		nullptr,                 /* �쐬����E�B���h�E�̐e�܂��̓I�[�i�[�E�B���h�E�̃n���h�����w�� */
		nullptr,                 /* �E�B���h�E�X�^�C���ɉ������q�E�B���h�EID�̎w��i����͎g�p���Ȃ��j*/
		m_hInst,                 /* �E�B���h�E�Ɋ֘A�t�����Ă���C���X�^���X�̃n���h����ݒ� */
		nullptr);                /* ��������E�B���h�E�ɓn���C�ӂ̃p�����[�^�[���w��i����͎g�p���Ȃ��j*/

	if (m_hWnd == nullptr) {
		return false;
	}


	// �E�B���h�E��\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);


	// �E�B���h�E�̍X�V
	UpdateWindow(m_hWnd);


	// �E�B���h�E�Ƀt�H�[�J�X
	SetFocus(m_hWnd);


	// ����I��
	return true;
}


bool App::InitDirect3D() {

	// �f�o�b�O���C���[�̒ǉ�
#if defined (DEBUG) || defined (_DEBUG)
	ComPtr<ID3D12Debug> debug;
	HRESULT Hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
	if (SUCCEEDED(Hr)) {

		debug->EnableDebugLayer();
	}
#endif

	// �f�o�C�X�̐���
	HRESULT hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf()));
	if (FAILED(hr)) {

		return false;
	}
	m_pDevice->SetName(L"GPUDevice");

#if defined (DEBUG) || defined (_DEBUG)
	m_pDevice.As(&debugDevice);
	
#endif


	// �R�}���h�L���[�̐���
	{
		D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
		CmdQueueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;       // GPU�ɃR�}���h�L���[�𒼐ڎ��s
		CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;  // �D��x�̓f�t�H���g
		CmdQueueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;        // �R�}���h�L���[�����i����̓f�t�H���g�j
		CmdQueueDesc.NodeMask = 0;                                    // GPU�� 1��Ȃ̂� 0

		hr = m_pDevice->CreateCommandQueue(
			&CmdQueueDesc,
			IID_PPV_ARGS(m_pCmdQueue.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}
		m_pCmdQueue->SetName(L"CommandQueue");
	}


	// �X���b�v�`�F�C���̐���
	{
		// DXGI�t�@�N�g���[�̐���
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) {

			return false;
		}

		// �X���b�v�`�F�C���̐ݒ�
		DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
		SwapChainDesc.BufferDesc.Width                   = m_Width;                                 /* �𑜓x�̉��� */
		SwapChainDesc.BufferDesc.Height                  = m_Height;                                /* �𑜓x�̏c�� */
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;                                       /* ���t���b�V�����[�g�̕��� */
		SwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;                                      /* ���t���b�V�����[�g�̕��q */
		SwapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;           /* �X�P�[�����O�͂Ȃ� */
		SwapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;    /* �������̏����͎w��Ȃ� */
		SwapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;              /* GPU�ɂ�RGBA8�r�b�g��n���i0����1�ɐ��K���j*/
		SwapChainDesc.SampleDesc.Count                   = 1;                                       /* �s�N�Z���P�ʂ̃}���`�T���v�����O���i����͕K�v�Ȃ��̂łP�j */
		SwapChainDesc.SampleDesc.Quality                 = 0;                                       /* �摜�̕i�����x����ݒ� */
		SwapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;         /* �o�b�N�o�b�t�@�̎g�p���@���w�� */
		SwapChainDesc.BufferCount                        = FrameCount;                              /* �o�b�t�@�̐��i�_�u���o�b�t�@�����O�̂��߂Q�j */
		SwapChainDesc.OutputWindow                       = m_hWnd;                                  /* �o�͂̃E�B���h�E�n���h�����w�� */
		SwapChainDesc.Windowed                           = TRUE;                                    /* �X���b�v�`�F�C�����E�B���h�E���[�h�œ������̐ݒ� */
		SwapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;           /* �X���b�v�`�F�C�����s���̓����i����͕ύX��ɔj���j*/
		SwapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  /* �X���b�v�`�F�C���̓���I�v�V���� */
	
		// �X���b�v�`�F�C���̐���
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(
			m_pCmdQueue.Get(),
			&SwapChainDesc,
			&pSwapChain);
		if (FAILED(hr)) {

			SafeRelease(pFactory);
			return false;
		}

		// �o�b�N�o�b�t�@�ԍ��̎擾�̂��߂�IDXGIFatory3 ���擾
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.GetAddressOf()));
		if (FAILED(hr)) {

			SafeRelease(pSwapChain);
			SafeRelease(pFactory);

			return false;
		}

		// �o�b�N�o�b�t�@�̔ԍ����擾
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();


		// �K�v�Ȃ��I�u�W�F�N�g�̉��
		SafeRelease(pSwapChain);
		SafeRelease(pFactory);

	}


	// �R�}���h�A���P�[�^�[�̐����i�t���[���J�E���g������������j
	{
		for (auto i = 0u; i < FrameCount; i++) {

			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));
			if (FAILED(hr)) {

				return false;
			}
		}

		m_pCmdAllocator[0]->SetName(L"CommandAllocator_1");
		m_pCmdAllocator[1]->SetName(L"CommandAllocator_2");
	}


	// �R�}���h���X�g�̐���
	{
		hr = m_pDevice->CreateCommandList(
			0,                                      /* �m�[�h�}�X�N�̐ݒ�i�����GPU�̐��� 1�Ȃ̂� 0�j */
			D3D12_COMMAND_LIST_TYPE_DIRECT,         /* �쐬����R�}���h���X�g�̃^�C�v��ݒ�B�i����̓R�}���h�L���[�ɒ��ړo�^���邽�� *DIRECT�j */
			m_pCmdAllocator[m_FrameIndex].Get(),    /* �R�}���h�A���P�[�^�[�̐ݒ� */
			nullptr,                                /* �p�C�v���C���X�e�[�g�̐ݒ�i���̌�ɖ����I�ɐݒ肷�邽�� nullptr�j */
			IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}

		m_pCmdList->SetName(L"CommandList");
	}


	// �����_�[�^�[�Q�b�g�r���[�̐���
	if (!m_DespManager.CreateRTV(m_pDevice.Get(), m_pSwapChain.Get())) {

		return false;
	}


	// �[�x�X�e���V���r���[�̐���
	if (!m_DespManager.CreateDSV(m_pDevice.Get(), m_Width, m_Height)) {

		return false;
	}


	// �t�F���X�̐���
	{
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,          /* �t�F���X�̋��L�̐ݒ�i����͋��L���Ȃ��j*/
			IID_PPV_ARGS(m_pFence.GetAddressOf()));
		if (FAILED(hr)) {

			return false;
		}

		m_pFence->SetName(L"Fence");
	}


	// �C�x���g�̐���
	{
		m_FenceEvent = CreateEvent(
			nullptr,                 /* �q�v���Z�X���擾�����n���h�����p���ł��邩���肷��\���̂ւ̃|�C���^���w�� */
			FALSE,                   /* ����͎����̃��Z�b�g�I�u�W�F�N�g��ݒ� */
			FALSE,                   /* �C�x���g�I�u�W�F�N�g�̏�����ԁi����͔�V�O�i����� FALSE ���w��j */
			nullptr);                /* �C�x���g�I�u�W�F�N�g�̖��O�̓o�^�i����͎g�p���Ȃ��j */
		if (m_FenceEvent == nullptr) {

			return false;
		}
	}


	// �R�}���h���X�g�����
	m_pCmdList->Close();

	// Imgui�̏�����
	if (!Init_Imgui()) {

		std::cout << "Error : Can't Initialize Imgui." << std::endl;
		return false;
	}


	// ����I��
	return true;
}



bool App::OnInit() {

	// Imgui�̏�����
	{
		if (ImGui::CreateContext() == nullptr) {

			std::cout << "Imgui�̏������Ɏ��s" << std::endl;
			return false;
		}

		// Window�̏�����
		bool blnResult = ImGui_ImplWin32_Init(m_hWnd);
		if (!blnResult) {

			std::cout << "Imgui��Window�������Ɏ��s" << std::endl;
			return false;
		}

		blnResult = ImGui_ImplDX12_Init(
			m_pDevice.Get(),                                         // �f�o�C�X
			3,                                                       // �t���[��
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,                         // RTV�t�H�[�}�b�g
			m_pHeapForImgui.Get(),                                   // Imgui�q�[�v
			m_pHeapForImgui->GetCPUDescriptorHandleForHeapStart(),   // CPU�n���h��
			m_pHeapForImgui->GetGPUDescriptorHandleForHeapStart());  // GPU�n���h��

		if (!blnResult) {

			std::cout << "Imgui�������Ɏ��s" << std::endl;
			return false;
		}

	}


	// �q�[�v�T�C�Y��ݒ�
	size_t size = 5;
	m_DespManager.Init_CBV_SRV_UAV(m_pDevice.Get(), size);

	//// ���b�V�������[�h
	//{

	//	// �p�X�̌���
	//	std::wstring path;
	//	if (!SearchFilePath(L"Floor/Untitled.obj", path)) {

	//		std::cout << "OBJ�t�@�C����������܂���" << std::endl;
	//		return false;
	//	}

	//	// ���b�V���̃��[�h
	//	if (!LoadMesh(path.c_str(), m_meshes, m_materials)) {

	//		std::cout << "OBJ�t�@�C�������[�h�ł��܂���" << std::endl;
	//		return false;
	//	}
	//}



	//// ���_�o�b�t�@�̐���
	//{

	//	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�i����͎w��Ȃ��j*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */


	//	// �o�b�t�@�T�C�Y������
	//	auto VertexSize = sizeof(MeshVertex) * m_meshes[0].Vertices.size();  /* MeshVertex�i���_���j�~ ���_���̐� */
	//	auto vertices = m_meshes[0].Vertices.data();                         /* �}�b�s���O�p�̒��_�f�[�^���m�ۂ���i�ϔz��̐擪�|�C���^���擾�j*/

	//	// ���\�[�X�̐ݒ�
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j*/
	//	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	//	resourceDesc.Width              = UINT64(VertexSize);               /* ���_��񂪓���T�C�Y�̃o�b�t�@�T�C�Y�i�e�N�X�`���̏ꍇ�͉������w��j*/
	//	resourceDesc.Height             = 1;                                /* �o�b�t�@�̏ꍇ�͂P�i�e�N�X�`���̏ꍇ�͏c�����w��j*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* ���\�[�X�̉��s�i�o�b�t�@�E�e�N�X�`���͂P�A�O�����e�N�X�`���͉��s�j*/
	//	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ� */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j*/

	//	// ���_�o�b�t�@�iGPU���\�[�X�j�𐶐�
	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,                                   /* �q�[�v�̐ݒ� */
	//		D3D12_HEAP_FLAG_NONE,                        /* �q�[�v�̃I�v�V���� */
	//		&resourceDesc,                               /* ���\�[�X�̐ݒ� */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,           /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
	//		nullptr,                                     /* RTV��DSV�p�̐ݒ� */
	//		IID_PPV_ARGS(m_pVB.GetAddressOf()));         /* �A�h���X���i�[ */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// ���_�o�b�t�@�̃}�b�s���O���s��
	//	void* ptr = nullptr;
	//	hr = m_pVB->Map(0, nullptr, &ptr);  //�@��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// ���_�f�[�^���}�b�s���O��ɐݒ�iGPU�̃��������R�s�[�j
	//	memcpy(ptr, vertices, VertexSize);

	//	// �}�b�s���O�̉���
	//	m_pVB->Unmap(0, nullptr);


	//	// ���_�o�b�t�@�r���[�̐ݒ�i���_�o�b�t�@�̕`��R�}���h�p�EGPU�̃A�h���X��T�C�Y�Ȃǂ��L�����Ă����j
	//	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();                          /* ��قǃ}�b�v����GPU�̉��z�A�h���X���L�� */
	//	m_VBV.SizeInBytes    = static_cast<UINT>(VertexSize);                          /* ���_�f�[�^�S�̂̃T�C�Y���L�� */
	//	m_VBV.StrideInBytes  = static_cast<UINT>(sizeof(MeshVertex));                  /* �P���_�ӂ�̃T�C�Y���L�� */
	//}




	//// �C���f�b�N�X�o�b�t�@�̐���
	//{
	//	

	//	// �q�[�v�v���p�e�B
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /*  */
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /*  */
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /*  */
	//	heapProp.CreationNodeMask     = 1;                                /*  */
	//	heapProp.VisibleNodeMask      = 1;                                /*  */


	//	// �C���f�b�N�X�o�b�t�@�̃T�C�Y������
	//	auto IndexSize = sizeof(uint32_t) * m_meshes[0].Indices.size();  /* �C���f�b�N�X��uint32_t�^ �~ �C���f�b�N�X�̐� */
	//	auto indices   = m_meshes[0].Indices.data();                     /* �}�b�s���O�p�C���f�b�N�X�f�[�^�̐擪�|�C���^���擾 */

	//	// ���\�[�X�̐ݒ�
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j*/
	//	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	//	resourceDesc.Width              = IndexSize;                        /* �C���f�b�N�X��񂪓���T�C�Y�̃o�b�t�@�T�C�Y�i�e�N�X�`���̏ꍇ�͉������w��j*/
	//	resourceDesc.Height             = 1;                                /* �o�b�t�@�̏ꍇ�͂P�i�e�N�X�`���̏ꍇ�͏c�����w��j*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* ���\�[�X�̉��s�i�o�b�t�@�E�e�N�X�`���͂P�A�O�����e�N�X�`���͉��s�j*/
	//	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i����̓o�b�t�@�Ȃ̂� *UNKNOWN�B�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ� */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j*/
	//	

	//	// ���\�[�X�̐���
	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,
	//		&resourceDesc,
	//		D3D12_RESOURCE_STATE_GENERIC_READ,
	//		nullptr,
	//		IID_PPV_ARGS(m_pIB.GetAddressOf()));
	//	if (FAILED(hr)) {
	//		std::cout << "�C���f�b�N�X�o�b�t�@�̃G���[" << std::endl;
	//		return false;
	//	}


	//	// �}�b�s���O���s��
	//	void* ptr = nullptr;
	//	hr = m_pIB->Map(0, nullptr, &ptr);
	//	if (FAILED(hr)) {

	//		return false;
	//	}

	//	// �C���f�b�N�X�f�[�^��GPU�������ɃR�s�[����
	//	memcpy(ptr, indices, IndexSize);


	//	// �}�b�s���O�̉���
	//	m_pIB->Unmap(0, nullptr);


	//	// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
	//	m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();  /* �C���f�b�N�X�o�b�t�@��GPU������ */
	//	m_IBV.Format         = DXGI_FORMAT_R32_UINT;           /* �t�H�[�}�b�g�i�|���S�����������ꍇ *R32_UINT �|���S���������Ȃ��ꍇ *R16_UINT�j */
	//	m_IBV.SizeInBytes    = static_cast<UINT>(IndexSize);   /* �C���f�b�N�X�f�[�^�̃f�[�^�T�C�Y�i�o�C�g�j*/
	//}



	//// CBV / SRV / UAV�p�̃f�B�X�N���v�^�q�[�v�̐���
	//{

	//	// �f�B�X�N���v�^�q�[�v�̐ݒ�
	//	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//	heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     /* �萔�o�b�t�@���܂񂾃t���O���w�� */
	//	heapDesc.NumDescriptors = 10;                                         /* �f�B�X�N���v�^�̐� */
	//	heapDesc.NodeMask       = 0;                                          /* GPU�͂P�Ȃ̂łO���w�� */
	//	heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  /* �V�F�[�_�[������Q�Ƃł���悤�ɂ��� */

	//	// �f�B�X�N���v�^�q�[�v�̐���
	//	HRESULT hr = m_pDevice->CreateDescriptorHeap(
	//		&heapDesc,
	//		IID_PPV_ARGS(m_pHeapCBV_SRV_UAV.GetAddressOf()));
	//	if (FAILED(hr)) {

	//		return false;
	//	}
	//}
	//


	//// �萔�o�b�t�@�̐����i���W�s��Ȃǂ��V�F�[�_�[�ɓn���o�b�t�@�j
	//{
	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                 // �A���C�����g�T�C�Y�i256 Byte�j
	//	uint64_t alignmentSize = (sizeof(Transform) + (align - 1)) & ~(align - 1);   // �A���C�����g�T�C�Y�̌v�Z

	//	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�iCPU�̏������ݕ��@�ɂ��āE����͎w��Ȃ��j*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */

	//	// ���\�[�X�̐ݒ�
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j */
	//	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	//	resourceDesc.Width              = alignmentSize;                    /* �f�[�^�T�C�Y�F256 Byte�i256 Byte�𒴂���ꍇ 512 Byte�j*/
	//	resourceDesc.Height             = 1;                                /* �f�[�^�̏c�̃T�C�Y�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* �f�[�^�̉��s�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ�i����͎g��Ȃ��̂łO�j */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �n�܂肩��I���܂ŘA�������o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j */


	//	// �f�B�X�N���v�^�q�[�v���̃f�[�^�ɂ��āA���̃f�[�^�Ɉړ����邽�߂̃C���N�������g�T�C�Y���擾
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	//	// �o�b�N�o�b�t�@�̐����̒萔�o�b�t�@�𐶐��iGPU���\�[�X�j
	//	for (auto i = 0u; i < FrameCount; i++) {

	//		HRESULT hr = m_pDevice->CreateCommittedResource(
	//			&heapProp,                               /* �q�[�v�̐ݒ� */
	//			D3D12_HEAP_FLAG_NONE,                    /* �q�[�v�̃I�v�V���� */
	//			&resourceDesc,                           /* ���\�[�X�̐ݒ� */
	//			D3D12_RESOURCE_STATE_GENERIC_READ,       /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
	//			nullptr,                                 /* RTV��DSV�p�̐ݒ� */
	//			IID_PPV_ARGS(m_pCB[i].GetAddressOf()));  /* �A�h���X���i�[ */
	//		if (FAILED(hr)) {

	//			return false;
	//		}

	//		// �萔�o�b�t�@�̃A�h���X
	//		auto addressGPU = m_pCB[i]->GetGPUVirtualAddress();                          // GPU�̉��z�A�h���X���擾
	//		auto handleCPU  = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  // �f�B�X�N���v�^�q�[�v�̐擪�n���h�����擾�iCPU�j
	//		auto handleGPU  = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  // �f�B�X�N���v�^�q�[�v�̐擪�n���h�����擾�iGPU�j

	//		// �萔�o�b�t�@�̐擪�|�C���^���v�Z�i�萔�o�b�t�@�̃T�C�Y�����C���N�������g�j
	//		handleCPU.ptr += incrementSize * i;
	//		handleGPU.ptr += incrementSize * i;

	//		// �萔�o�b�t�@�r���[�̐ݒ�i�萔�o�b�t�@�̏���ۑ��j
	//		m_CBV[i].HandleCPU              = handleCPU;          // �萔�o�b�t�@�̐擪�n���h���iCPU�j
	//		m_CBV[i].HandleGPU              = handleGPU;          // �萔�o�b�t�@�̐擪�n���h���iGPU�j
	//		m_CBV[i].CBVDesc.BufferLocation = addressGPU;         // �o�b�t�@�̕ۑ��ʒu���w��
	//		m_CBV[i].CBVDesc.SizeInBytes    = alignmentSize;      // �萔�o�b�t�@�̃T�C�Y

	//		// �萔�o�b�t�@�r���[�̐���
	//		m_pDevice->CreateConstantBufferView(&m_CBV[i].CBVDesc, handleCPU);


	//		//�萔�o�b�t�@�iTransform�j�̃}�b�s���O���s��
	//		hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));  // ��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	//		if (FAILED(hr)) {
	//			std::cout << "�萔�o�b�t�@�̃}�b�v�G���[" << std::endl;
	//			return false;
	//		}

	//		// �}�b�s���O�����s���ݒ肷��
	//		auto eyePos = DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);     /* �J�������W */
	//		auto targetPos = DirectX::XMVectorZero();                       /* �����_���W�i���_�j*/
	//		auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);     /* �J�����̍��� */

	//		auto fovY = DirectX::XMConvertToRadians(37.5f);                           /* �J������ Y���ɑ΂����p */
	//		auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height); /* �����ɑ΂��镝�̊��� */

	//		// �ϊ��s��̐ݒ�
	//		m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity() * DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);                                          /* ���[���h�s��EIdentity�͒P�ʍs�� */
	//		m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);                  /* �J�����s�� */
	//		m_CBV[i].pBuffer->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);  /* �ˉe�s�� */
	//	}

	//	m_CBV[0].pBuffer->World *= DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//	m_CBV[1].pBuffer->World *= DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//}


	//// ���C�g�̍쐬
	//{
	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                    // �A���C�����g�T�C�Y�i256 Byte�j
	//	uint64_t alignmentSize = (sizeof(LightBuffer) + (align - 1)) & ~(align - 1);    // �A���C�����g�T�C�Y�̌v�Z

	//	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�iCPU�̏������ݕ��@�ɂ��āE����͎w��Ȃ��j*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */

	//	// ���\�[�X�̐ݒ�
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j */
	//	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	//	resourceDesc.Width              = alignmentSize;                    /* �f�[�^�T�C�Y�F256 Byte�i256 Byte�𒴂���ꍇ 512 Byte�j*/
	//	resourceDesc.Height             = 1;                                /* �f�[�^�̏c�̃T�C�Y�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* �f�[�^�̉��s�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ�i����͎g��Ȃ��̂łO�j */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �n�܂肩��I���܂ŘA�������o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j */


	//	// �f�B�X�N���v�^�q�[�v���̃f�[�^�ɂ��āA���̃f�[�^�Ɉړ����邽�߂̃C���N�������g�T�C�Y���擾
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,                    /* �q�[�v�̃I�v�V���� */
	//		&resourceDesc,                           /* ���\�[�X�̐ݒ� */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,       /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
	//		nullptr,                                 /* RTV��DSV�p�̐ݒ� */
	//		IID_PPV_ARGS(m_pCB[2].GetAddressOf()));  /* �A�h���X���i�[ */
	//	if (FAILED(hr)) {

	//		std::cout << "Error : Can't create Material Resource." << std::endl;
	//		return false;
	//	}

	//	auto virtualAddress = m_pCB[2]->GetGPUVirtualAddress();                          /* GPU�̃A�h���X */
	//	auto handleCPU      = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  /* CPU�q�[�v�̐擪 */
	//	auto handleGPU      = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  /* GPU�q�[�v�̐擪 */

	//	// �s��ϊ��p��CBV�����΂�
	//	handleCPU.ptr += incrementSize * 2;
	//	handleGPU.ptr += incrementSize * 2;

	//	// �萔�o�b�t�@�r���[
	//	m_LightCBV.HandleCPU              = handleCPU;         /* CPU���z�A�h���X */
	//	m_LightCBV.HandleGPU              = handleGPU;         /* GPU�q�[�v�̐擪 */
	//	m_LightCBV.CBVDesc.BufferLocation = virtualAddress;    /* GPU���z�A�h���X */
	//	m_LightCBV.CBVDesc.SizeInBytes    = static_cast<UINT>(alignmentSize);     /* �o�b�t�@�̃T�C�Y�i�A���C�����g�T�C�Y�j*/

	//	// �萔�o�b�t�@�r���[�̐���
	//	m_pDevice->CreateConstantBufferView(&m_LightCBV.CBVDesc, handleCPU);


	//	//�萔�o�b�t�@�iMaterial�j�̃}�b�s���O���s��
	//	hr = m_pCB[2]->Map(0, nullptr, reinterpret_cast<void**>(&m_LightCBV.pBuffer));  // ��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	//	if (FAILED(hr)) {
	//		std::cout << "���C�g�p�萔�o�b�t�@�̃}�b�v�G���[" << std::endl;
	//		return false;
	//	}

	//	m_LightCBV.pBuffer->LightPosition = Vector4(10.0f, 0.0f, 0.0f, 0.0f);  /* ���C�g���W */
	//	m_LightCBV.pBuffer->LightColor    = Color(1.0f, 1.0f, 1.0f, 0.0f);    /* ���C�g�F */
	//}


	//// �}�e���A���̐����iCBV�j
	//{

	//	auto align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;                 // �A���C�����g�T�C�Y�i256 Byte�j
	//	uint64_t alignmentSize = (sizeof(Material) + (align - 1)) & ~(align - 1);    // �A���C�����g�T�C�Y�̌v�Z

	//	// �q�[�v�v���p�e�B�i�f�[�^���ǂ�����̂����L�q�j
	//	D3D12_HEAP_PROPERTIES heapProp = {};
	//	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;           /* �q�[�v�̃^�C�v���w��i�����GPU�ɑ���p�̃q�[�v�j*/
	//	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  /* CPU�y�[�W�v���p�e�B�iCPU�̏������ݕ��@�ɂ��āE����͎w��Ȃ��j*/
	//	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;        /* �������v�[���̈����ɂ��āi����͎w��Ȃ��j*/
	//	heapProp.CreationNodeMask     = 1;                                /* GPU�̐� */
	//	heapProp.VisibleNodeMask      = 1;                                /* GPU�̎��ʂ��鐔 */

	//	// ���\�[�X�̐ݒ�
	//	D3D12_RESOURCE_DESC resourceDesc = {};
	//	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;  /* �������\�[�X�̎�����ݒ�i���_�o�b�t�@�Ȃ̂� *BUFFER���w��j */
	//	resourceDesc.Alignment          = 0;                                /* �������̋�؂�� *BUFFER�̏ꍇ�� 64 KB�܂��� 0���w�� */
	//	resourceDesc.Width              = alignmentSize;                    /* �f�[�^�T�C�Y�F256 Byte�i256 Byte�𒴂���ꍇ 512 Byte�j*/
	//	resourceDesc.Height             = 1;                                /* �f�[�^�̏c�̃T�C�Y�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.DepthOrArraySize   = 1;                                /* �f�[�^�̉��s�i�o�b�t�@�Ȃ̂łP�j*/
	//	resourceDesc.MipLevels          = 1;                                /* �~�b�v�}�b�v�̃��x���̐ݒ�i�o�b�t�@�̏ꍇ�͂P�j */
	//	resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;              /* �f�[�^�̃t�H�[�}�b�g���w��i�e�N�X�`���̏ꍇ�̓s�N�Z���t�H�[�}�b�g���w��j*/
	//	resourceDesc.SampleDesc.Count   = 1;                                /* �A���`�G�C���A�V���O�̐ݒ�i�O���ƃf�[�^���Ȃ����ƂɂȂ��Ă��܂��j*/
	//	resourceDesc.SampleDesc.Quality = 0;                                /* �A���`�G�C���A�V���O�̐ݒ�i����͎g��Ȃ��̂łO�j */
	//	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;   /* �n�܂肩��I���܂ŘA�������o�b�t�@�Ȃ̂� *MAJOR�i�e�N�X�`���̏ꍇ�� *UNKNOWN�j*/
	//	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;         /* ����͐ݒ�Ȃ��iRTV�EDSV�EUAV�ESRV�̏ꍇ�͐ݒ肷��j */


	//	// �f�B�X�N���v�^�q�[�v���̃f�[�^�ɂ��āA���̃f�[�^�Ɉړ����邽�߂̃C���N�������g�T�C�Y���擾
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	HRESULT hr = m_pDevice->CreateCommittedResource(
	//		&heapProp,
	//		D3D12_HEAP_FLAG_NONE,                    /* �q�[�v�̃I�v�V���� */
	//		&resourceDesc,                           /* ���\�[�X�̐ݒ� */
	//		D3D12_RESOURCE_STATE_GENERIC_READ,       /* ���\�[�X�̏�����Ԃ��w��i�q�[�v�ݒ�� *UPLOAD�ɂ����ꍇ *GENERIC_READ���w��j*/
	//		nullptr,                                 /* RTV��DSV�p�̐ݒ� */
	//		IID_PPV_ARGS(m_pCB[3].GetAddressOf()));  /* �A�h���X���i�[ */
	//	if (FAILED(hr)) {

	//		std::cout << "Error : Can't create Material Resource." << std::endl;
	//		return false;
	//	}

	//	auto virtualAddress = m_pCB[3]->GetGPUVirtualAddress();                          /* GPU�̃A�h���X */
	//	auto handleCPU      = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();  /* CPU�q�[�v�̐擪 */
	//	auto handleGPU      = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();  /* GPU�q�[�v�̐擪 */

	//	// �s��ϊ��p��CBV�����΂�
	//	handleCPU.ptr += incrementSize * 3;
	//	handleGPU.ptr += incrementSize * 3;

	//	// �萔�o�b�t�@�r���[
	//	m_Material.HandleCPU              = handleCPU;         /* CPU���z�A�h���X */
	//	m_Material.HandleGPU              = handleGPU;         /* GPU�q�[�v�̐擪 */
	//	m_Material.CBVDesc.BufferLocation = virtualAddress;    /* GPU���z�A�h���X */
	//	m_Material.CBVDesc.SizeInBytes    = static_cast<UINT>(alignmentSize);     /* �o�b�t�@�̃T�C�Y�i�A���C�����g�T�C�Y�j*/

	//	// �萔�o�b�t�@�r���[�̐���
	//	m_pDevice->CreateConstantBufferView(&m_Material.CBVDesc, handleCPU);


	//	//�萔�o�b�t�@�iMaterial�j�̃}�b�s���O���s��
	//	hr = m_pCB[3]->Map(0, nullptr, reinterpret_cast<void**>(&m_Material.pBuffer));  // ��Őݒ肵���T�C�Y����GPU���\�[�X�ւ̃|�C���^���擾
	//	if (FAILED(hr)) {
	//		std::cout << "�}�e���A���p�萔�o�b�t�@�̃}�b�v�G���[" << std::endl;
	//		return false;
	//	}

	//	// ���b�V������萔�o�b�t�@���擾
	//	Material material = m_materials[0];

	//	// �}�e���A���̐ݒ�
	//	m_Material.pBuffer->Diffuse   = material.Diffuse;
	//	m_Material.pBuffer->alpha     = material.alpha;
	//	m_Material.pBuffer->Specular  = material.Specular;
	//	m_Material.pBuffer->Shininess = material.Shininess;

	//	std::cout << "Diffuse : " << m_Material.pBuffer->Diffuse.x << ", " << m_Material.pBuffer->Diffuse .y << ", " << m_Material.pBuffer->Diffuse.z << std::endl;
	//	std::cout << "alpha : " << m_Material.pBuffer->alpha << std::endl;
	//	std::cout << "Specular : " << m_Material.pBuffer->Specular.x << ", " << m_Material.pBuffer->Specular.y << ", " << m_Material.pBuffer->Specular.z << std::endl;
	//	std::cout << "Shininess : " << m_Material.pBuffer->Shininess << std::endl;
	//}



	//// �e�N�X�`���̐����iSRV�j
	//{
	//	// �t�@�C���p�X�̌���
	//	std::wstring texturePath;
	//	if (!SearchFilePath(L"Floor/BrickRound.dds", texturePath)) {

	//		return false;
	//	}


	//	// �摜�f�[�^�̓ǂݎ�菈��
	//	DirectX::ResourceUploadBatch batch(m_pDevice.Get());
	//	batch.Begin();                                        // ���������ŃR�}���h�A���P�[�^�[�ƃR�}���h���X�g�����������

	//	// ���\�[�X�̐����i���̊֐��ň�C�Ƀ��\�[�X�𐶐��ł���B���\�[�X�ݒ肩�炵�Ȃ��Ă��悢�j
	//	HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//		m_pDevice.Get(),                     /* �f�o�C�X��n�� */
	//		batch,                               /*  */
	//		texturePath.c_str(),                 /* �e�N�X�`���܂ł̃p�X�i���C�h�������}���`�o�C�g�ɕϊ����ēn���K�v����j*/
	//		m_Texture.pResource.GetAddressOf(),  /* �e�N�X�`���̃A�h���X���擾 */
	//		true);                               /*  */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// �R�}���h�̎��s
	//	auto future = batch.End(m_pCmdQueue.Get());

	//	// �R�}���h�����܂őҋ@
	//	future.wait();

	//	// �C���N�������g�T�C�Y���擾�i�V�F�[�_�[�p�iSRV�j�̃������̃C���N�������g�T�C�Y�j
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	// CPU�f�B�X�N���v�^�n���h����GPU�f�B�X�N���v�^�n���h�����f�B�X�N���v�^�q�[�v����擾
	//	auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
	//	auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

	//	// �e�N�X�`���Ƀf�B�X�N���v�^�n���h�������蓖�Ă�iCBV�~4���΂��K�v����j
	//	handleCPU.ptr += incrementSize * 4;
	//	handleGPU.ptr += incrementSize * 4;

	//	m_Texture.HandleCPU = handleCPU;
	//	m_Texture.HandleGPU = handleGPU;

	//	// �e�N�X�`���̍\�����擾�i��̃��C�u�����Ő������ꂽ�\����ǂݍ��ށj
	//	auto textureDesc = m_Texture.pResource->GetDesc();

	//	// �V�F�[�_���\�[�X�r���[�̐ݒ�
	//	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	//	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	//	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	//	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	//	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	//	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	//	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	//	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	//	// �V�F�[�_�[���\�[�X�r���[�̐���
	//	m_pDevice->CreateShaderResourceView(
	//		m_Texture.pResource.Get(),         /*  */
	//		&SRVDesc,                          /*  */
	//		handleCPU);                        /*  */
	//}


	//// �@���}�b�v�̐����iSRV�j
	//{
	//	// �t�@�C���p�X�̌���
	//	std::wstring texturePath;
	//	if (!SearchFilePath(L"Floor/BrickRoundBUMP.dds", texturePath)) {

	//		std::cout << "Error : Can't find Texture." << std::endl;
	//		return false;
	//	}


	//	// �摜�f�[�^�̓ǂݎ�菈��
	//	DirectX::ResourceUploadBatch batch(m_pDevice.Get());
	//	batch.Begin();                                        // ���������ŃR�}���h�A���P�[�^�[�ƃR�}���h���X�g�����������

	//	// ���\�[�X�̐����i���̊֐��ň�C�Ƀ��\�[�X�𐶐��ł���B���\�[�X�ݒ肩�炵�Ȃ��Ă��悢�j
	//	HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//		m_pDevice.Get(),                        /* �f�o�C�X��n�� */
	//		batch,                                  /*  */
	//		texturePath.c_str(),                    /* �e�N�X�`���܂ł̃p�X�i���C�h�������}���`�o�C�g�ɕϊ����ēn���K�v����j*/
	//		NormalMapSRV.pResource.GetAddressOf(),  /* �e�N�X�`���̃A�h���X���擾 */
	//		true);                                  /*  */
	//	if (FAILED(hr)) {

	//		return false;
	//	}


	//	// �R�}���h�̎��s
	//	auto future = batch.End(m_pCmdQueue.Get());

	//	// �R�}���h�����܂őҋ@
	//	future.wait();

	//	// �C���N�������g�T�C�Y���擾�i�V�F�[�_�[�p�iSRV�j�̃������̃C���N�������g�T�C�Y�j
	//	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	// CPU�f�B�X�N���v�^�n���h����GPU�f�B�X�N���v�^�n���h�����f�B�X�N���v�^�q�[�v����擾
	//	auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
	//	auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

	//	// �e�N�X�`���Ƀf�B�X�N���v�^�n���h�������蓖�Ă�iCBV�~4�{SRV�~1���΂��K�v����j
	//	handleCPU.ptr += incrementSize * 5;
	//	handleGPU.ptr += incrementSize * 5;

	//	NormalMapSRV.HandleCPU = handleCPU;
	//	NormalMapSRV.HandleGPU = handleGPU;

	//	// �e�N�X�`���̍\�����擾�i��̃��C�u�����Ő������ꂽ�\����ǂݍ��ށj
	//	auto textureDesc = NormalMapSRV.pResource->GetDesc();

	//	// �V�F�[�_���\�[�X�r���[�̐ݒ�
	//	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	//	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	//	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	//	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	//	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	//	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	//	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	//	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	//	// �V�F�[�_�[���\�[�X�r���[�̐���
	//	m_pDevice->CreateShaderResourceView(
	//		NormalMapSRV.pResource.Get(),        /*  */
	//		&SRVDesc,                            /*  */
	//		handleCPU);                          /*  */
	//}


	// ���������̏�������̃I�u�W�F�N�g�N���X�ɏ����Ƃ��Ă܂Ƃ߂���==============================================

	if (!model.Init(m_pDevice.Get(), m_pCmdQueue.Get(), &m_DespManager, L"Floor/untitled.obj")){

		return false;
	}



	// ���[�g�V�O�l�`�������i�V�F�[�_�[���Ŏg�p���郊�\�[�X�̈����������߂�j
	{

		// ���[�g�V�O�l�`���̃��C�A�E�g�I�v�V�����̐ݒ�i�_���a�Ŏw��E�g�p���Ȃ����̂��L�q�j
		auto flagLayout = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;     /* �n���V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X������ */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;   /* �h���C���V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X������ */
		flagLayout |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; /* �W�I���g���[�V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X������ */


		// �f�B�X�N���v�^�[�e�[�u���iSRV�j�͈̔͂�ݒ�

		// �e�N�X�`����SRV
		D3D12_DESCRIPTOR_RANGE range[2] = {};
		range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  /* �f�B�X�N���v�^�̎�ނ�ݒ� */
		range[0].NumDescriptors                    = 1;                                /* �f�B�X�N���v�^�[�̐� */
		range[0].BaseShaderRegister                = 0;                                /* �n�߂郌�W�X�^�ԍ� */
		range[0].RegisterSpace                     = 0;                                /*  */
		range[0].OffsetInDescriptorsFromTableStart = 0;                                /* �I�t�Z�b�g */

		// �@���}�b�v��SRV
		range[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  /* �f�B�X�N���v�^�̎�� */
		range[1].NumDescriptors                    = 1;                                /* �f�B�X�N���v�^�̐� */
		range[1].BaseShaderRegister                = 1;                                /* �n�߂�ԍ� */
		range[1].RegisterSpace                     = 0;                                /*  */
		range[1].OffsetInDescriptorsFromTableStart = 0;                                /* �I�t�Z�b�g */


		// ���[�g�p�����[�^�[�̐ݒ�iCBV�ESRV���V�F�[�_�[�ɑ��� | CBV | CBV | CBV | SRV | SRV | �j

		// �ϊ��s��iCBV�j
		D3D12_ROOT_PARAMETER rootParam[5] = {};
		rootParam[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;  /* ���[�g�p�����[�^�[�̃^�C�v�͒萔�o�b�t�@�iCBV�j*/
		rootParam[0].Descriptor.ShaderRegister = 0;                              /* ���W�X�^�̊J�n�ԍ� */
		rootParam[0].Descriptor.RegisterSpace  = 0;                              /* ����͎g��Ȃ� */
		rootParam[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX; /* ���_�V�F�[�_�[����Q�Ƃł���悤�ɂ��� */

		// ���C�g�s��iCBV�j
		rootParam[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[1].Descriptor.ShaderRegister = 1;
		rootParam[1].Descriptor.RegisterSpace  = 0;
		rootParam[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// �}�e���A���s��iCBV�j
		rootParam[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[2].Descriptor.ShaderRegister = 2;
		rootParam[2].Descriptor.RegisterSpace  = 0;
		rootParam[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// �e�N�X�`���p�iSRV�j
		rootParam[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  /* ���[�g�p�����[�^�[�̃^�C�v�i�f�B�X�N���v�^�[�e�[�u���j*/
		rootParam[3].DescriptorTable.NumDescriptorRanges = 1;                                           /* �����W�̐� */
		rootParam[3].DescriptorTable.pDescriptorRanges   = &range[0];                                   /* �ݒ肵���f�B�X�N���v�^�����W���w�� */
		rootParam[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               /* �s�N�Z���V�F�[�_�[�Ŏg�p�ł���悤�ɂ��� */

		// �@���}�b�v�p�iSRV�j
		rootParam[4].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  
		rootParam[4].DescriptorTable.NumDescriptorRanges = 1;                                           
		rootParam[4].DescriptorTable.pDescriptorRanges   = &range[1];                                   
		rootParam[4].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               


		// �X�^�e�B�b�N�T���v���[�̐ݒ�
		D3D12_STATIC_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;              /* �e�N�X�`�����T���v�����O����ۂ̃t�B���^�����O���@ */
		SamplerDesc.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* U�����ɑ΂���e�N�X�`���A�h���b�V���O */
		SamplerDesc.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* V�����ɑ΂���e�N�X�`���A�h���b�V���O */
		SamplerDesc.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;             /* W�����ɑ΂���e�N�X�`���A�h���b�V���O */
		SamplerDesc.MipLODBias       = D3D12_DEFAULT_MIP_LOD_BIAS;                   /* �v�Z���ꂽ�~�b�v���x���̃I�t�Z�b�g�i�v�Z��R�F�o�C�A�X�F�Q�̏ꍇ�A�K�������̂͂T�j */
		SamplerDesc.MaxAnisotropy    = 1;                                            /* �t�B���^�����O��ANISOTROPIC�̏ꍇ�̂ݗL�� */
		SamplerDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;                  /* �����̃T���v���f�[�^�ɑ΂��ăT���v�����ꂽ�f�[�^�̔�r�ݒ�i����͂Ȃ��j */
		SamplerDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  /* �A�h���b�V���O���[�h�� *_BORDER�̏ꍇ�̋��E���̐F��ݒ� */
		SamplerDesc.MinLOD           = -D3D12_FLOAT32_MAX;                           /* �~�b�v�}�b�v�͈͂̉����l��ݒ� */
		SamplerDesc.MaxLOD           = +D3D12_FLOAT32_MAX;                           /* �~�b�v�}�b�v�͈͂̏���l��ݒ� */
		SamplerDesc.ShaderRegister   = 0;                                            /* HLSL�̃o�C���f�B���O�ɑΉ����郌�W�X�^��ݒ� */
		SamplerDesc.RegisterSpace    = 0;                                            /* ���W�X�^��Ԃ̐ݒ� */
		SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                /* �s�N�Z���V�F�[�_�[�Ŏg�p�ł���悤�ɐݒ� */

		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.NumParameters     = _countof(rootParam);  /* ���[�g�p�����[�^�[�̐� */
		rootSigDesc.NumStaticSamplers = 1;                    /* �X�^�e�B�b�N�T���v���[�̐� */
		rootSigDesc.pParameters       = rootParam;            /* ��`�������[�g�p�����[�^�[���w��i�z��̐擪�A�h���X�j*/
		rootSigDesc.pStaticSamplers   = &SamplerDesc;         /* ��قǐݒ肵���X�^�e�B�b�N�T���v���[���w�� */
		rootSigDesc.Flags             = flagLayout;           /* �w�肵�����̂����[�g�V�O�l�`�����A�N�Z�X���邱�Ƃ����ۂ���t���O */

		// ���[�g�V�O�l�`���̃V���A���C�Y�i�o�C�g��ɕϊ��j
		ComPtr<ID3DBlob> pBlob;                     /* ���[�g�V�O�l�`�����o�C�g�ϊ��������̂�ۑ� */
		ComPtr<ID3DBlob> pErrorBlob;                /* �V���A���C�Y�Ɏ��s���̃G���[���e */

		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,                           /* ��قǐݒ肵�����[�g�V�O�l�`�����w�� */
			D3D_ROOT_SIGNATURE_VERSION_1_0,         /* ���[�g�V�O�l�`���̃o�[�W�����w�� */
			pBlob.GetAddressOf(),                   /* �V���A���C�Y���������[�g�V�O�l�`����ۑ� */
			pErrorBlob.GetAddressOf());             /* �V���A���C�Y�Ɏ��s�����ꍇ�ɃG���[���e���������܂�� */
		if (FAILED(hr)) {

			std::cout << "���[�g�V�O�l�`���V���A���G���[" << std::endl;
			return false;
		}

		// ���[�g�V�O�l�`���̐���
		hr = m_pDevice->CreateRootSignature(
			0,                                               /* �m�[�h�}�X�N�̐ݒ�iGPU���P�����Ȃ̂łO�j */
			pBlob->GetBufferPointer(),                       /* �V���A���C�Y�����f�[�^�ւ̃|�C���^ */
			pBlob->GetBufferSize(),                          /* �V���A���C�Y�����f�[�^�̃T�C�Y */
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf()));
		if (FAILED(hr)) {
			std::cout << "���[�g�V�O�l�`�������G���[" << std::endl;
			return false;
		}
	}



	// �p�C�v���C���X�e�[�g�̐���
	{

		// ���X�^���C�U�[�X�e�[�g�̐ݒ�i�|���S���f�[�^����s�N�Z���f�[�^�ւƃ��X�^���C�Y���s���ۂ̏�Ԑݒ�j
		D3D12_RASTERIZER_DESC RSdesc = {};
		RSdesc.FillMode              = D3D12_FILL_MODE_SOLID;                      /* SOLID�F���g���h��EWIREFRAME�F���C���[�̂݁i���_�Ԃ����Ԑ��̂݁j*/
		RSdesc.CullMode              = D3D12_CULL_MODE_NONE;                       /* �J�����O�����̐ݒ�i�w�肵�������ɑ�����ʂ̕`������Ȃ��悤�ɂ���ݒ�j */
		RSdesc.FrontCounterClockwise = FALSE;                                      /* �S�ʂ����v��肩�����v��肩���w�� */
		RSdesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;                   /* �^�����t�s�N�Z���ɉ��Z����[�x�l��ݒ�B�i���s�𒲐����邽�߂̂��́B����͎g�p���Ȃ��j */
		RSdesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;             /* �s�N�Z���ő�[�x�o�C�A�X�l��ݒ�i����͎g�p���Ȃ��j */
		RSdesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;      /* �s�N�Z���̌��z�ɉ������[�x�o�C�A�X�l���X�P�[������X�J���[�l��ݒ�i����͎g�p���Ȃ��j */
		RSdesc.DepthClipEnable       = FALSE;                                      /* �����Ɋ�N���b�s���O��L���ɂ��邩���w�� */
		RSdesc.MultisampleEnable     = FALSE;                                      /* �}���`�T���v�����O�i�A���`�G�C���A�V���O�j��L���ɂ��邩���w�� */
		RSdesc.AntialiasedLineEnable = FALSE;                                      /* ���̃A���`�G�C���A�V���O�̐ݒ� */
		RSdesc.ForcedSampleCount     = 0;                                          /* �A���I�[�_�[�A�N�Z�X�r���[�̕`��܂��̓��X�^���C�U�̊ԁA�T���v�����������I�ɌŒ�l�ɂ���i����͋������Ȃ��̂łO�j */
		RSdesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;  /* ���X�^���C�U�[�ŏ����ł����̃s�N�Z�������ɂ������Ă����炻�̃s�N�Z�������X�^������ */


		// �u�����h�X�e�[�g�̐ݒ�i���͗v�f�E���Z�v�f�E�o�͗v�f�̂R���ǂ̂悤�ɍ������邩��ݒ�E�������̏������ł���j

		// �܂������_�[�^�[�Q�b�g�̃u�����h��ݒ�
		D3D12_RENDER_TARGET_BLEND_DESC RTVBlendDesc = {
			FALSE,                          /* �u�����f�B���O��L���ɂ��邩 */
			FALSE,                          /* �_�����Z��L���ɂ��邩 */
			D3D12_BLEND_ONE,                /* RGB  �s�N�Z���V�F�[�_�[����o�͂��ꂽRGB�l�ɑ΂���u�����h�I�v�V�������w�� */
			D3D12_BLEND_ZERO,               /* RGB  ���݂̃����_�[�^�[�Q�b�g��RGB�l�ɑ΂��Ď��s����u�����h�I�v�V�������w�� */
			D3D12_BLEND_OP_ADD,             /* ���� ��̓���ǂ̂悤�Ɍ������邩���`����I�v�V���� */
			D3D12_BLEND_ONE,                /* �� �s�N�Z���V�F�[�_�[����o�͂��ꂽ�A���t�@�l�ɑ΂��Ď��s����I�v�V�������w�� */
			D3D12_BLEND_ZERO,               /* �� ���݂̃����_�[�^�[�Q�b�g�̃A���t�@�l�ɑ΂��Ď��s����I�v�V�������w�� */
			D3D12_BLEND_OP_ADD,             /* ���� �����ǂ̂悤�Ɍ������邩���`����I�v�V���� */
			D3D12_LOGIC_OP_NOOP,            /* �����_�[�^�[�Q�b�g�ɑ΂��Đݒ肷��_�����Z���w��i����͑������s��Ȃ��̂� *NOOP�j */
			D3D12_COLOR_WRITE_ENABLE_ALL    /* �����_�[�^�[�Q�b�g�̏������݃}�X�N���w��i����͂��ׂĂ̒l�ɑ΂��ď������݂��s���j */
		};

		// �u�����h�X�e�[�g�̐ݒ�
		D3D12_BLEND_DESC BlendStateDesc = {};
		BlendStateDesc.AlphaToCoverageEnable  = FALSE;                         /* �s�N�Z���V�F�[�_�[�̏o�̓A���t�@�l�������擾���A�A���`�G�C���A�V���O������L���ɂ���i�g�p���Ȃ��j */
		BlendStateDesc.IndependentBlendEnable = FALSE;                         /* FALSE�̏ꍇ���̃����_�[�^�[�Q�b�g��0�݂̂ɂȂ�i�P�`�V�͖��������j */
		for (auto i = 0u; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {   /* �����_�[�^�[�Q�b�g�������i�_�u���o�b�t�@�p�������j����ꍇ */
			BlendStateDesc.RenderTarget[i]    = RTVBlendDesc;
		}



		// �V�F�[�_�[�̐ݒ�
		ComPtr<ID3DBlob> pVSBlob;  /* ���_�V�F�[�_�[ */
		ComPtr<ID3DBlob> pPSBlob;  /* �s�N�Z���V�F�[�_�[ */


		// �R���p�C�����ꂽ�V�F�[�_�[��ǂݍ���
		std::wstring vsPath;
		std::wstring psPath;

		if (!SearchFilePath(L"VertexShader.cso", vsPath)) {

			return false;
		}

		if (!SearchFilePath(L"PixelShader.cso", psPath)) {

			return false;
		}


		// ���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[��ǂݍ��ށi�R���p�C���ς݂̃V�F�[�_�[��ǂݍ��ށBcso : compiled sheder object�j
		HRESULT hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr)) {

			return false;
		}

		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr)) {

			return false;
		}


		// �[�x�X�e���V���X�e�[�g�̐ݒ�
		D3D12_DEPTH_STENCIL_DESC DSSdesc = {};
		DSSdesc.DepthEnable    = TRUE;                              /* �[�x�e�X�g���s�����ǂ��� */
		DSSdesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;        /* �[�x�X�e���V���o�b�t�@�ւ̏������݂�L���ɂ��� */
		DSSdesc.DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;  /* ��r�̍ۂ̊֐��i����́u�ȉ��v��ݒ�j */
		DSSdesc.StencilEnable  = FALSE;                             /* �X�e���V���e�X�g���s�����ǂ��� */



		// �p�C�v���C���X�e�[�g�̐���
		D3D12_GRAPHICS_PIPELINE_STATE_DESC GPdesc = {};
		GPdesc.InputLayout                     = MeshVertex::InputLayout;                                    /* ���͗v�f�̐ݒ� */
		GPdesc.pRootSignature                  = m_pRootSignature.Get();                                     /* ���[�g�V�O�l�`���̐ݒ� */
		GPdesc.VS                              = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };  /* ���_�V�F�[�_�[���w�� */
		GPdesc.PS                              = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };  /* �s�N�Z���V�F�[�_�[���w�� */
		GPdesc.RasterizerState                 = RSdesc;                                                     /* ���X�^���C�U�[�̐ݒ���w�� */
		GPdesc.BlendState                      = BlendStateDesc;                                             /* �u�����h�X�e�[�g�̐ݒ���w�� */
		GPdesc.DepthStencilState               = DSSdesc;                                                    /* �[�x�X�e���V���̐ݒ� */
		GPdesc.SampleMask                      = UINT_MAX;                                                   /* �u�����h��Ԃ̃T���v���}�X�N */
		GPdesc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;                     /* �O�p�`����͂Ƃ��Ĉ��� */
		GPdesc.NumRenderTargets                = 1;                                                          /* RTVFormats�����o�[���̃����_�[�^�[�Q�b�g�`���̐� */
		GPdesc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                            /* �����_�[�^�[�Q�b�g�`���� */
		GPdesc.DSVFormat                       = DXGI_FORMAT_D32_FLOAT;                                      /* �[�x�X�e���V���r���[�̃t�H�[�}�b�g���w��i����͎g�p���Ȃ��j */
		GPdesc.SampleDesc.Count                = 1;                                                          /* �}���`�T���v�����O�̎w��i�A���`�G�C���A�X���������O���Ɖ������Ȃ��̂łP�j */
		GPdesc.SampleDesc.Quality              = 0;                                                          /* �}���`�T���v�����O�̕i�����w��i�A���`�G�C���A�X�����͎g�p���Ȃ��̂łO�j */


		// �p�C�v���C���X�e�[�g�̐���
		hr = m_pDevice->CreateGraphicsPipelineState(
			&GPdesc,
			IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr)) {

			std::cout << "�p�C�v���C���X�e�[�g�̃G���[" << std::endl;
			return false;
		}
	}


	// �r���[�|�[�g�ƃV�U�[��`�̐ݒ�
	{
		m_Viewport.TopLeftX = 0;                             /* �r���[�|�[�g�̂����W�̊ */
		m_Viewport.TopLeftY = 0;                             /* �r���[�|�[�g�̂����W�̊ */
		m_Viewport.Width    = static_cast<float>(m_Width);   /* �r���[�|�[�g�̕��iint����float�ɃL���X�g�j*/
		m_Viewport.Height   = static_cast<float>(m_Height);  /* �r���[�|�[�g�̍����iint����float�ɃL���X�g�j*/
		m_Viewport.MinDepth = 0.0f;                          /* �r���[�|�[�g�̍ŏ��[�x */
		m_Viewport.MaxDepth = 1.0f;                          /* �r���[�|�[�g�̍ő�[�x */

		m_Scissor.left   = 0;                                 /* ������̈�̍� x���W */
		m_Scissor.right  = m_Width;                           /* ������̈�̉E x���W */
		m_Scissor.top    = 0;                                 /* ������̈�̏� y���W */
		m_Scissor.bottom = m_Height;                          /* ������̈�̉� y���W */
	}
	

	// ����I��
	return true;
}


void App::MainLoop() {

	// ���b�Z�[�W����ۑ�����ϐ�
	MSG msg = {};

	/* �I�����b�Z�[�W�����M�����܂Ń��[�v������ */
	while (WM_QUIT != msg.message) {

		/* ���b�Z�[�W���󂯎�����珈������ */
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {

			TranslateMessage(&msg);  // �L�[���b�Z�[�W�𕶎����b�Z�[�W�ɕϊ�
			DispatchMessage(&msg);   // �E�B���h�E�v���V�[�W���ɑ��M
		}
		else {

			Update();   // �X�V����
			Render();   // �`�揈��
		}
	}

}


void App::Update() {

	model.Update();

}


void App::Render() {

	// �R�}���h�̋L�^���J�n����
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(
		m_pCmdAllocator[m_FrameIndex].Get(),    /* �R�}���h�A���P�[�^�[�̎w�� */
		nullptr);                               /* �p�C�v���C���X�e�[�g�̐ݒ� */


	// ���\�[�X�o���A�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;       /* ���\�[�X�o���A�̃^�C�v���w��i����͑J�ځj*/
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;             /* �t���O�̎w�� */
	barrier.Transition.pResource   = m_DespManager.GetResource_RTB(m_FrameIndex);  /* �o���A���s�����\�[�X�̐擪�A�h���X */
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;                 /* �O��ԁF�\�� */
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;           /* ���ԁF�����_�[�^�[�Q�b�g */
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;      /* �S�ẴT�u���\�[�X����x�ɑJ�ڂ����� */

	// �ݒ肵�����\�[�X�o���A�����蓖�Ă�
	m_pCmdList->ResourceBarrier(1, &barrier);



	// RTV�EDSV�̐ݒ�
	{
		auto handleRTV = m_DespManager.GetCPUHandle_RTV(m_FrameIndex);
		auto handleDSV = m_DespManager.GetCPUHandle_DSV();
		m_pCmdList->OMSetRenderTargets(
			1,                            /* �ݒ肷�郌���_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�n���h���̐����w�� */
			&handleRTV,                   /* �ݒ肷�郌���_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�n���h���̔z��i�|�C���^�j���w�� */
			FALSE,                        /* �f�B�X�N���v�^�n���h�������L�����邩(TRUE) �Ɨ������邩(FALSE) ���w��i�قƂ�ǂ̏ꍇ�͓Ɨ�������j */
			&handleDSV);                  /* �ݒ肷��[�x�X�e���V���r���[�i���s�����L������o�b�t�@�̃r���[�j��ݒ肷�� */

		// �N���A�J���[�̐ݒ�
		float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };


		// �����_�[�^�[�Q�b�g�r���[���N���A
		m_pCmdList->ClearRenderTargetView(
			handleRTV,                       /* �����_�[�^�[�Q�b�g�r���[���N���A���邽�߂̃f�B�X�N���v�^�n���h�����w�� */
			clearColor,                      /* �����_�[�^�[�Q�b�g���w��̐F�ŃN���A���邽�߂̐F���w�� */
			0,                               /* �ݒ肷���`�̐����w��i����͎g�p���Ȃ��j*/
			nullptr);                        /* �����_�[�^�[�Q�b�g���N���A���邽�߂̋�`�̔z����w�肷��inullptr�̏ꍇ�S�̂��N���A�����j */
	

		// �[�x�X�e���V���r���[���N���A
		m_pCmdList->ClearDepthStencilView(m_DespManager.GetCPUHandle_DSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}


	// �`�揈��
	{
		auto heap = m_DespManager.GetHeapCBV_SRV_UAV();
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());     /* ���[�g�V�O�l�`���𑗐M */
		m_pCmdList->SetDescriptorHeaps(1, &heap);                         /* CBV�ESRV�EUAV�𑗐M */
		m_pCmdList->SetPipelineState(m_pPSO.Get());                       /* �p�C�v���C���X�e�[�g�̐ݒ� */

		m_pCmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   /* ���͂��O�p�`�v�f�ł��邱�Ƃ��w�� */
		m_pCmdList->RSSetViewports(1, &m_Viewport);                                  /* �r���[�|�[�g�̐ݒ� */
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);                                /* �V�U�[��`�̐ݒ� */

		// ���b�V���̕`��
		model.Render(m_pCmdList.Get(), m_FrameIndex);
	}

	// Imgui�̕`��
	ImguiRender();


	// �o���A��Present��ԁi�\����ԁj�ɐݒ肷��
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource   = m_DespManager.GetResource_RTB(m_FrameIndex);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;      /* �O��ԁF�����_�[�^�[�Q�b�g */
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;            /* ���ԁF�\�� */
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// �ݒ肵���o���A�����蓖�Ă�
	m_pCmdList->ResourceBarrier(1, &barrier);


	// �R�}���h���X�g�̋L�^���I��
	m_pCmdList->Close();


	// �R�}���h�̎��s�i�R�}���h�L���[�Ŏ��s�A�R�}���h���X�g�𕡐��ݒ肷�邱�Ƃ��\�j
	ID3D12CommandList* CmdLists[] = { m_pCmdList.Get() };
	m_pCmdQueue->ExecuteCommandLists(
		1,                             /* �R�}���h���X�g�̐����w��i�����̃R�}���h���X�g��o�^�\�j */
		CmdLists);                     /* �R�}���h���X�g���܂Ƃ߂��z�� */


	// ��ʂɕ\��
	Present(1);
}


void App::Present(uint32_t interval) {

	// ��ʂɕ\��
	m_pSwapChain->Present(interval, 0);


	// �V�O�i������
	const uint64_t currentCount = m_FenceCounter[m_FrameIndex];
	m_pCmdQueue->Signal(m_pFence.Get(), currentCount);


	// �o�b�N�o�b�t�@�ԍ��̍X�V
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();


	// ���̃t���[���̕`�揀�����܂��̏ꍇ�͑ҋ@�i�t�F���X�̒l���X�V����Ă��Ȃ���Αҋ@�j
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex]) {

		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}


	// ���̃t���[���̃t�F���X�J�E���^�[�𑝂₷
	m_FenceCounter[m_FrameIndex] = currentCount + 1;

}


void App::WaitGPU() {

	// �|�C���^�� NULL�̏ꍇ�G���[
	assert(m_pCmdQueue  != nullptr);
	assert(m_pFence     != nullptr);
	assert(m_FenceEvent != nullptr);


	// �V�O�i���̏���
	m_pCmdQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);


	// �������ɃC�x���g�ݒ肷��
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);


	//�@GPU�̏��������ҋ@
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);


	// �J�E���^�[�𑝂₷
	m_FenceCounter[m_FrameIndex]++;
}


void App::TermApp() {

	// �`��C���^�[�t�F�C�X�̔j��
	OnTerm();

	// Direct3D�̔j��
	TermDirect3D();

	// �E�B���h�E�̔j��
	TermWindow();

}


void App::OnTerm() {


	m_pPSO.Reset();              // �p�C�v���C���X�e�[�g�̔j��

	//// �e�N�X�`���̔j��
	//m_Texture.pResource.Reset();
	//m_Texture.HandleCPU.ptr = 0;
	//m_Texture.HandleGPU.ptr = 0;

	//// �@���}�b�v�̔j��
	//NormalMapSRV.pResource.Reset();
	//NormalMapSRV.HandleCPU.ptr = 0;
	//NormalMapSRV.HandleGPU.ptr = 0;


	//// �萔�o�b�t�@��j������
	//for (auto i = 0u; i < _countof(m_pCB); i++) {

	//	if (m_pCB[i] != nullptr){

	//		m_pCB[i]->Unmap(0, nullptr);             // �A���}�b�v
	//		memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
	//	}
	//	m_pCB[i].Reset();
	//}

	//// ���b�V���̔j��
	//for (size_t i = 0; i < m_meshes.size(); i++) {

	//	m_meshes[i].Vertices.clear();
	//	m_meshes[i].Indices.clear();
	//}
	//m_meshes.clear();

	//// �}�e���A���̍폜
	//m_materials.clear();


	//// �o�b�t�@�̔j��
	//m_pIB.Reset();
	//m_pVB.Reset();


	//m_pHeapCBV_SRV_UAV.Reset();  // CBV�ESRV�EUAV�̃f�B�X�N���v�^�q�[�v��j��


	//// �o�b�t�@�r���[�̔j��
	//m_VBV.BufferLocation = 0;
	//m_VBV.SizeInBytes    = 0;
	//m_VBV.StrideInBytes  = 0;

	//m_IBV.BufferLocation = 0;
	//m_IBV.SizeInBytes    = 0;
	//m_IBV.Format         = DXGI_FORMAT_UNKNOWN;

	// ���f���̔j��
	model.Term();


	// ���[�g�V�O�l�`���̔j��
	m_pRootSignature.Reset();

}


void App::TermDirect3D() {

	// GPU�̏�������������܂őҋ@
	WaitGPU();


	/* �����������ԂƋt���ɉ�����Ă��� */


	// Imgui�̔j��
	m_pHeapForImgui.Reset();
	Term_Imgui();

	
	// �C�x���g�̔j��
	if (m_FenceEvent != nullptr) {

		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}


	// �t�F���X�̔j��
	m_pFence.Reset();


	// RTV�EDSV�̔j��
	m_DespManager.Term();


	// �R�}���h���X�g�̔j��
	m_pCmdList.Reset();


	// �R�}���h�A���P�[�^�[�̔j��
	for (auto i = 0u; i < FrameCount; i++) {

		m_pCmdAllocator[i].Reset();
	}


	// �X���b�v�`�F�C���̔j��
	m_pSwapChain.Reset();


	// �R�}���h�L���[�̔j��
	m_pCmdQueue.Reset();


	// �f�o�C�X�̔j��
	m_pDevice.Reset();

#if defined (DEBUG) || defined (_DEBUG)

	/* ���̃��|�[�g���f�o�C�X���Q�Ƃ��Ă���̂ňȉ��̃��|�[�g�ɂ�Device���j������Ă��Ȃ��Əo�� */
	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
	debugDevice.Reset();
#endif
}



void App::TermWindow() {

	// �E�B���h�E�o�^�̉���
	if (m_hInst != nullptr) {
		UnregisterClass(m_windowTitle, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}


// ImGui�p�E�B���h�E�v���V�[�W��
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
		
		/* �E�B���h�E���j�������ꍇ�ɑ��M����� */
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			break;
	}

	// ImGui�̏������s��
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);


	/* ���̃��b�Z�[�W���������Ă����֐���Ԃ� */
	return DefWindowProc(hwnd, msg, wp, lp);
}


template<typename T>
void SafeRelease(T* ptr) {

	if (ptr != nullptr) {

		ptr->Release();
		ptr = nullptr;
	}
}


/* imgui�̊֐��͈ȉ��ɒ�` */

// Imgui ������
bool App::Init_Imgui() {

	// �f�B�X�N���v�^�q�[�v�̎擾
	m_pHeapForImgui = CreateDescriptorHeapForImgui();

	if (m_pHeapForImgui == nullptr) {

		return false;
	}

	return true;
}


// Imgui�pDescriptorHeap�̐���
ComPtr<ID3D12DescriptorHeap> App::CreateDescriptorHeapForImgui() {

	ComPtr<ID3D12DescriptorHeap> ret;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  /* �V�F�[�_�[���猩����悤�ɐݒ� */
	desc.NodeMask       = 0;                                          /* ������GPU�͂Ȃ� */
	desc.NumDescriptors = 1;                                          /* �q�[�v�̐��͂P�� */
	desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     /* CBV_SRV_UAV�Œ�` */

	HRESULT hr = m_pDevice->CreateDescriptorHeap(
		&desc, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));

	return ret;
}

// �f�B�X�N���v�^�q�[�v�̎Q�Ə���
ComPtr<ID3D12DescriptorHeap> App::GetHeapForImgui() {

	return m_pHeapForImgui;
}


// Imgui�̕`��
void App::ImguiRender() {

	// �`��O�����i�P�̃E�B���h�E��NewFrame���R�K�v�j
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// �E�B���h�E�̒�`
	ImGui::Begin("Render Test Menu");
	ImGui::SetWindowSize(ImVec2(300, 400), ImGuiCond_::ImGuiCond_FirstUseEver);
	ImGui::End();

	ImGui::Render();
	m_pCmdList->SetDescriptorHeaps(1, m_pHeapForImgui.GetAddressOf());      // Imgui�̃q�[�v���Z�b�g
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pCmdList.Get());  // �R�}���h���Z�b�g
}

// Imgui�̏I������
void App::Term_Imgui() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}