//コンスタントバッファで行列を転送
#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>

#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>

#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;

///@brief コンソール画面にフォーマット付き文字列を表示
///@param format フォーマット
///@param 可変長引数
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12Device* _dev = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

#ifdef _DEBUG
int main() {
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int) {
#endif
	DebugOutputFormatString("Show window test.");
	HINSTANCE hInst = GetModuleHandle(nullptr);
	//ウィンドウクラス生成＆登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// コールバック関数の指定
	w.lpszClassName = _T("DirectXTex");			// アプリケーションクラス名
	w.hInstance = GetModuleHandle(0);			// ハンドルの取得
	RegisterClassEx(&w);						// アプリケーションクラス

	RECT wrc = { 0,0,window_width, window_height };		// ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	// ウィンドウのサイズを関数を使って補正する
	// ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(
		w.lpszClassName,			// クラス名指定
		_T("DX12テスト 3D座標"),	// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,		// タイトルバーと境界線
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

#ifdef _DEBUG
	// デバッグレイヤーをオンに
	EnableDebugLayer();
#endif

	//DirectX12初期化
	//フィーチャレベル列挙
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

	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = l;
			break;
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;				// タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// プライオリティ特に指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;				// ここはコマンドリストと合わせる
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));	// コマンドキュー

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

	result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// レンダーターゲットビューなので当然RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						// 表裏の２つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	// SRGBレンダーターゲットビュー設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	// これやると色味がよくなるが、バックバッファとの
	// フォーマットの食い違いによりDebugLayerにエラーが出力される
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
		//rtvDesc.Format = _backBuffers[i]->GetDesc().Format;
		_dev->CreateRenderTargetView(_backBuffers[i], &rtvDesc, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);// ウィンドウ表示

	struct Vertex {
		XMFLOAT3 pos;	//XYZ座標
		XMFLOAT2 uv;	//UV座標
	};

	// 頂点
	Vertex vertices[] = {
		{{-1.f,-1.0f,0.0f},	{0.0f,1.0f}},	// 左下
		{{-1.f,1.f,0.0f},	{0.0f,0.0f}},	// 左上
		{{1.f,-1.f,0.0f},	{1.0f,1.0f}},	// 右下
		{{1.f,1.f,0.0f},	{1.0f,0.0f}},	// 右上
	};

	// 頂点バッファの作成
	// UPLOAD(確保は可能)
	ID3D12Resource* vertBuff = nullptr;
	auto vertHeapProp	= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
	result = _dev->CreateCommittedResource(
		&vertHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	// 作ったバッファに頂点データをコピー
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();	// バッファの仮想アドレス
	vbView.SizeInBytes = sizeof(vertices);// 全バイト数
	vbView.StrideInBytes = sizeof(vertices[0]);	// 1頂点あたりのバイト数

	// インデックス
	unsigned short indices[] = { 0,1,2,2,1,3 };

	// インデックスバッファーの作成
	ID3D12Resource* idxBuff = nullptr;
	auto indexHeapProp		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto indexBufferDesc	= CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
	// 設定は、バッファのサイズ以外頂点バッファの設定を使いまわしてOK
	result = _dev->CreateCommittedResource(
		&indexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	// 作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);


	// シェーダ―
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// 頂点シェーダーの読み込み
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
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
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	// ピクセルシェーダーの読み込み
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
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipeline.BlendState.RenderTarget->BlendEnable = true;
	gpipeline.BlendState.RenderTarget->SrcBlend = D3D12_BLEND_SRC_ALPHA;
	gpipeline.BlendState.RenderTarget->DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	gpipeline.BlendState.RenderTarget->BlendOp = D3D12_BLEND_OP_ADD;

	gpipeline.RasterizerState.MultisampleEnable = false;		// アンチエイリアシングを使用しない
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	// 中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable = true;			// 深度方向のクリッピングを有効にする

	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	
	gpipeline.DepthStencilState.DepthEnable = false;
	gpipeline.DepthStencilState.StencilEnable = false;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;		// レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);	// レイアウト配列数

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	// ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 三角形で構成
	
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	// 0~1に正規化されたRGBA

	gpipeline.SampleDesc.Count = 1;	// サンプリングは１ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;

	ID3D12RootSignature* rootsignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};	// テクスチャと定数の２つ
	// テクスチャ用
	descTblRange[0].NumDescriptors = 1;	//	テクスチャひとつ
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// 種別はテクスチャ
	descTblRange[0].BaseShaderRegister = 0;	// 0番スロットから
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	// 定数用ディスクリプタレンジ
	descTblRange[1].NumDescriptors = 1;	//	定数ひとつ
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
	descTblRange[1].BaseShaderRegister = 0;	// 0番スロットから
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange[0];	// ディスクリプタレンジ
	rootparam.DescriptorTable.NumDescriptorRanges = 2;	// ディスクリプタレンジ数
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// 全てのシェーダーから見える

	rootSignatureDesc.pParameters = &rootparam;	// ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = 1;		// ルートパラメータ数

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			// 横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			// 縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;			// 奥繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	// ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;			// 補間しない（ニアレストネイバー）
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;							// ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;										// ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		// オーバーサンプリングの際リサンプリングしない
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// ピクセルシェーダーからのみ可視

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));

	gpipeline.pRootSignature = rootsignature;
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;		// 出力先の幅（ピクセル数）
	viewport.Height = window_height;	// 出力先の高さ（ピクセル数）
	viewport.TopLeftX = 0;				// 出力先の左上座標X
	viewport.TopLeftY = 0;				// 出力先の左上座標Y
	viewport.MaxDepth = 1.0f;			// 深度最大値
	viewport.MaxDepth = 0.0f;			// 深度最小値

	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;	// 切り抜き上座標
	scissorrect.left = 0;	// 切り抜き左座標
	scissorrect.right = scissorrect.left + window_width;	// 切り抜き右座標
	scissorrect.bottom = scissorrect.top + window_height;	// 切り抜き下座標

	//WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	result = LoadFromWICFile(L"img/textest.png", WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);	//生データ抽出

	// WriteToSubresourceで転送する用のヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;	// 特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	// ライトバック
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;	// 転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;	// 単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;	// 単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = metadata.format;	// DXGI_FORMAT_R8G8B8A8_UNORM;//RGBAフォーマット
	resDesc.Width = metadata.width;		// 幅
	resDesc.Height = metadata.height;	// 高さ
	resDesc.DepthOrArraySize = metadata.arraySize;	// 2Dで配列でもないので1
	resDesc.SampleDesc.Count = 1;	// 通常テクスチャなのでアンチエイリアシングしない
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = metadata.mipLevels;	// ミップマップしないのでミップ数１
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);	// 2Dテクスチャ用
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;	// とくにフラグなし

	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,	// テクスチャ用（ピクセルシェーダから見える）
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	result = texbuff->WriteToSubresource(
		0,
		nullptr,		// 全領域へコピー
		img->pixels,	// 元データアドレス
		img->rowPitch,	// 1ラインサイズ
		img->slicePitch	// 全サイズ
	);

	// 定数バッファ作成
	auto worldMat = XMMatrixRotationY(XM_PIDIV4);
	XMFLOAT3 eye(0, 0, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);
	auto projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,	//画角は90°
		static_cast<float>(window_width) / static_cast<float>(window_height),	//アス比
		1.0f,
		10.0f
	);
	ID3D12Resource* constBuff = nullptr;
	auto heapPropTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto matirxCBufferDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);
	result = _dev->CreateCommittedResource(
		&heapPropTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&matirxCBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);
	XMMATRIX* mapMatrix;	// マップ先を示すポインタ
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);	//マップ
	*mapMatrix = worldMat * viewMat * projMat;

	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// シェーダ―から見える
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;	// SRV１つとCBV１つ
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// 通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format;	// DXGI_FORMAT_R8B8G8A8_UNORM;//RGBA(0.0f~1.0fに正規化)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;	// ミップマップは使用しないので１

	// ディスクリプタの先頭アドレスを取得
	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	
	// シェーダ―リソースビューの作成
	_dev->CreateShaderResourceView(
		texbuff,	// ビューと関連付けるバッファ
		&srvDesc,	// テクスチャ設定情報
		basicHeapHandle
	);
	basicHeapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation	= constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes		= constBuff->GetDesc().Width;
	//定数バッファビューの作成
	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	MSG msg = {};
	unsigned int frame = 0;
	float angle = 0.0f;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		angle += 0.1f;
		worldMat = XMMatrixRotationY(angle);
		*mapMatrix = worldMat * viewMat * projMat;

		// DirectX処理
		// バックバッファのインデックスを取得
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		auto barrierPresentToRT = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		_cmdList->ResourceBarrier(1,&barrierPresentToRT);

		_cmdList->SetPipelineState(_pipelinestate);

		// レンダーターゲットを指定
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		// 画面クリア
		float r, g, b;
		r = (float)(0xff & frame >> 16) / 255.0f;
		g = (float)(0xff & frame >> 8) / 255.0f;
		b = (float)(0xff & frame >> 0) / 255.0f;
		float clearColor[] = { r,g,b,1.0f };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		++frame;

		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->SetGraphicsRootSignature(rootsignature);
		
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);

		_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->SetDescriptorHeaps(1, &basicDescHeap);
		_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		
		auto barrierRTToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		_cmdList->ResourceBarrier(1,&barrierRTToPresent);

		//命令のクローズ
		_cmdList->Close();

		// コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// 待ち
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		_cmdAllocator->Reset();	// キューをクリア
		_cmdList->Reset(_cmdAllocator, _pipelinestate);

		// フリップ
		_swapchain->Present(1, 0);



	}
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}

