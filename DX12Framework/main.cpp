//�|���S���\��
#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>

#include<d3dcompiler.h>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;

//@brief		�R���\�[����ʂɃt�H�[�}�b�g�t���������\��
//@param format �t�H�[�}�b�g(%d�Ƃ�%f) 
//@param		�ϒ�����
//@remarks		�R���\�[����ʂɃt�H�[�}�b�g�t���������\��
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

//�E�B���h�E�v���V�[�W��
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {//�E�B���h�E���j�����ꂽ��Ă΂�܂�
		PostQuitMessage(0);//OS�ɑ΂��ďI����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//�K���̏������s��
}

const unsigned int window_width		= 1280;
const unsigned int window_height	= 720;

IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12Device* _dev = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
#endif
	DebugOutputFormatString("Show window test.");

	HINSTANCE hInst = GetModuleHandle(nullptr);
	// �E�B���h�E�N���X����&�o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DirectXTest");		// �A�v���P�[�V�����N���X��
	w.hInstance = GetModuleHandle(0);			// �n���h���̎擾
	RegisterClassEx(&w);						// �A�v���P�[�V�����N���X

	RECT wrc = { 0,0,window_width,window_height };		// �E�B���h�E�T�C�Y�����߂�
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	// �E�B���h�E�̃T�C�Y��␳
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(
		w.lpszClassName,				// �N���X���w��
		_T("DX12 �P���|���S���e�X�g"),	// �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,			// �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,					// �\��X���W��OS�ɈϔC
		CW_USEDEFAULT,					// �\��Y���W��OS�ɈϔC
		wrc.right - wrc.left,			// �E�B���h�E��
		wrc.bottom - wrc.top,			// �E�B���h�E��
		nullptr,						// �e�E�B���h�E�n���h��
		nullptr,						// ���j���[�n���h��
		w.hInstance,					// �Ăяo���A�v���P�[�V�����n���h��
		nullptr							// �ǉ��p�����[�^
	);

#ifdef _DEBUG
	// �f�o�b�O���C���[���I��
	EnableDebugLayer();
#endif

	// DirectX12������
	// �t�B�[�`�����x����
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}

	// Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = l;
			break;
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	//_cmdList->Close();
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;							// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;				// �v���C�I���e�B���Ɏw��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;							// �����̓R�}���h���X�g�ƍ��킹��
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));	// �R�}���h�L���[����

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// �����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						// �\���̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// ���Ɏw��Ȃ�
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < swcDesc.BufferCount; ++i) {
		//IID3D12Resource* backBuffer = nullptr;
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW); // �E�B���h�E�\��

	XMFLOAT3 vertices[] = {
		{-0.4f,-0.7f,0.0f}, // ����
		{-0.4f, 0.7f,0.0f}, // ����
		{ 0.4f,-0.7f,0.0f}, // �E��
		{ 0.4f, 0.7f,0.0f}, // �E��
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// UPLOAD(�m�ۂ͉\)
	ID3D12Resource* vertBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();	// �o�b�t�@�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);						// �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);					// 1���_������̃o�C�g��

	unsigned short indices[] = { 0,1,2, 2,1,3 };

	ID3D12Resource* idxBuff = nullptr;
	// �ݒ�́A�o�b�t�@�̃T�C�Y�ȊO���_�o�b�t�@�̐ݒ���g���܂킵��OK
	resdesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	// ������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob,
		&errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);	// �����I��
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob,
		&errorBlob);

	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);	// �����I��
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 0xffffffff

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	// ���Z/��Z/���u�����f�B���O���g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// �_�����Z���g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.RasterizerState.MultisampleEnable = false;		// �A���`�F�����g��Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// �J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	// ���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;			// �[�x�����̃N���b�s���O�͗L��

	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias				= D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp		= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias	= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable	= false;
	gpipeline.RasterizerState.ForcedSampleCount		= 0;
	gpipeline.RasterizerState.ConservativeRaster	= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	gpipeline.DepthStencilState.DepthEnable = false;
	gpipeline.DepthStencilState.StencilEnable = false;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;		// ���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout);	// ���C�A�E�g�z��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	// �X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// �O�p�`�ō\��

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	// 0�`1�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1;		// �T���v�����O��1�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;	// �N�I���e�B�͍Œ�

	ID3D12RootSignature* rootsignature = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;		// �o�͐�̕�(�s�N�Z����)
	viewport.Height = window_height;	// �o�͐�̍���(�s�N�Z����)
	viewport.TopLeftX = 0;				// �o�͐�̍�����WX
	viewport.TopLeftY = 0;				// �o�͐�̍�����WY
	viewport.MaxDepth = 1.0f;			// �[�x�ő�l
	viewport.MinDepth = 0.0f;			// �[�x�ŏ��l

	D3D12_RECT scissorrect = {};
	scissorrect.top		= 0;								// �؂蔲������W
	scissorrect.left	= 0;								// �؂蔲�������W
	scissorrect.right	= scissorrect.left + window_width;	// �؂蔲���E���W
	scissorrect.bottom	= scissorrect.top + window_height;	// �؂蔲�������W

	MSG msg = {};
	unsigned int frame = 0;
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// �A�v���P�[�V�����I��
		if (msg.message == WM_QUIT) {
			break;
		}

		// DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		_cmdList->ResourceBarrier(1, &BarrierDesc);

		_cmdList->SetPipelineState(_pipelinestate);

		// �����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		// ��ʃN���A

		float r, g, b;
		r = (float)(0xff & frame >> 16) / 255.0f;
		g = (float)(0xff & frame >> 8) / 255.0f;
		b = (float)(0xff & frame >> 0) / 255.0f;
		float clearColor[] = { r,g,b,1.0f };	// ���F
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		++frame;
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->SetGraphicsRootSignature(rootsignature);

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);

		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		// ���߂̃N���[�Y
		_cmdList->Close();

		// �R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// �ҋ@
		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		_cmdAllocator->Reset();							// �L���[���N���A
		_cmdList->Reset(_cmdAllocator, _pipelinestate);	// �ĂуR�}���h���X�g�����߂鏀��
		
		// �t���b�v
		_swapchain->Present(1, 0);

	}

	// �E�B���h�E�N���X�o�^����
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}
