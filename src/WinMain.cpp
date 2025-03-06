
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

	// ���̓f�o�C�X�̏�����
	if (!input.InitDirectInput(m_hInst)) {

		return false;
	}
	if (!input.InitKeyBoard(m_hWnd)) {

		return false;
	}
	if (!input.InitMouse(m_hWnd)) {

		return false;
	}


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


	// �O���[�o���f�B�X�N���v�^�q�[�v�̍쐬
	{
		size_t globalSize = 2;  // �J�����̐�����
		m_DespManager.Init_GlobalHeap(m_pDevice.Get(), globalSize);
	}


	// �J�����̐ݒ�
	{
		if (!camera.Init(m_pDevice.Get(), &m_DespManager)) {

			return false;
		}
	}


	// �q�[�v�T�C�Y�̐ݒ�
	size_t size = 20;
	m_DespManager.Init_CBV_SRV_UAV(m_pDevice.Get(), size);

	
	// ���f���̃��[�h
	if (!model[0].Init(m_pDevice.Get(), m_pCmdQueue.Get(), &m_DespManager, L"house/FarmhouseOBJ.obj")) {

		return false;
	}
	//if (!model[1].Init(m_pDevice.Get(), m_pCmdQueue.Get(), &m_DespManager, L"house/FarmhouseOBJ.obj")) {

	//	return false;
	//}
	//model[0].ModelScaling(Vector3(0.8f, 0.8f, 0.8f));
	//model[0].ModelTranslation(Vector3(0.0f, -1.0f, 0.0f));
	model[0].ModelScaling(Vector3(0.05f, 0.05f, 0.05f));  // ���f���̃X�P�[���ύX



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
		D3D12_ROOT_PARAMETER rootParam[6] = {};

		// �J�����s��
		rootParam[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;  /* ���[�g�p�����[�^�[�̃^�C�v�͒萔�o�b�t�@�iCBV�j*/
		rootParam[0].Descriptor.ShaderRegister = 0;                              /* ���W�X�^�̊J�n�ԍ� */
		rootParam[0].Descriptor.RegisterSpace  = 0;                              /* ����͎g��Ȃ� */
		rootParam[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX; /* ���_�V�F�[�_�[����Q�Ƃł���悤�ɂ��� */

		// �ϊ��s��iCBV�j
		rootParam[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV; 
		rootParam[1].Descriptor.ShaderRegister = 1;                             
		rootParam[1].Descriptor.RegisterSpace  = 0;                             
		rootParam[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;

		// ���C�g�s��iCBV�j
		rootParam[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[2].Descriptor.ShaderRegister = 2;
		rootParam[2].Descriptor.RegisterSpace  = 0;
		rootParam[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// �}�e���A���s��iCBV�j
		rootParam[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[3].Descriptor.ShaderRegister = 3;
		rootParam[3].Descriptor.RegisterSpace  = 0;
		rootParam[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

		// �e�N�X�`���p�iSRV�j
		rootParam[4].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  /* ���[�g�p�����[�^�[�̃^�C�v�i�f�B�X�N���v�^�[�e�[�u���j*/
		rootParam[4].DescriptorTable.NumDescriptorRanges = 1;                                           /* �����W�̐� */
		rootParam[4].DescriptorTable.pDescriptorRanges   = &range[0];                                   /* �ݒ肵���f�B�X�N���v�^�����W���w�� */
		rootParam[4].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               /* �s�N�Z���V�F�[�_�[�Ŏg�p�ł���悤�ɂ��� */

		// �@���}�b�v�p�iSRV�j
		rootParam[5].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  
		rootParam[5].DescriptorTable.NumDescriptorRanges = 1;                                           
		rootParam[5].DescriptorTable.pDescriptorRanges   = &range[1];                                   
		rootParam[5].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;               


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

	// �L�[�̍X�V
	input.Update();


	// �J�����̍X�V
	camera.Update(&input);


	//if (input.HoldKey(DIK_A)) {

	//	model[0].Update(m_FrameIndex);
	//}
	//if (input.HoldKey(DIK_D)) {
	//	model[1].ModelRotation(0.03f);
	//}
	
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
		ID3D12DescriptorHeap* heap = m_DespManager.GetHeapCBV_SRV_UAV();   /* �q�[�v�̎擾 */
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());      /* ���[�g�V�O�l�`���𑗐M */
		m_pCmdList->SetDescriptorHeaps(1u, &heap);
		m_pCmdList->SetPipelineState(m_pPSO.Get());                        /* �p�C�v���C���X�e�[�g�̐ݒ� */
		m_pCmdList->RSSetViewports(1, &m_Viewport);                        /* �r���[�|�[�g�̐ݒ� */
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);                      /* �V�U�[��`�̐ݒ� */

		// ���b�V���̕`��
		for (auto i = 0u; i < _countof(model); i++) {

			// �J�����̃Z�b�g
			m_pCmdList->SetGraphicsRootConstantBufferView(0, camera.GetVirtualAddress(m_FrameIndex));

			// ���f���̕`��
			model[0].Render(m_pCmdList.Get(), m_FrameIndex);
		}
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


	// ���f���̔j��
	for (auto i = 0u; i < _countof(model); i++) {

		model[i].Term();
	}

	// �J�����̔j��
	camera.Term();


	// ���[�g�V�O�l�`���̔j��
	m_pRootSignature.Reset();

}


void App::TermDirect3D() {

	// GPU�̏�������������܂őҋ@
	WaitGPU();


	/* �����������ԂƋt���ɉ�����Ă��� */

	// ���̓f�o�C�X�̔j��
	input.TermInputDevice();

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

	// �`�F�b�N�{�^��
	static bool blnChk = false;
	ImGui::Checkbox("CheckBoxTest", &blnChk);

	// ���W�I�{�^��
	static int radio = 0;
	ImGui::RadioButton("Radio 1", &radio, 0);
	ImGui::RadioButton("Radio 2", &radio, 1);

	// int�^�X���C�_�[
	static int nSlider = 0;
	ImGui::SliderInt("Int Slider", &nSlider, 0, 100);

	// float�^�X���C�_�[
	static float fSlider = 0.0f;
	ImGui::SliderFloat("float Slider", &fSlider, 0.0f, 100.0f);

	// �x�N�g���X���C�_�[
	static float fSlider_3[3] = {};
	ImGui::SliderFloat3("float3 Slider", fSlider_3, 0.0f, 100.0f);

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