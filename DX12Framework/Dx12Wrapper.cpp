#include "Dx12Wrapper.h"
#include<cassert>
#include<d3dx12.h>
#include"Application.h"

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

namespace {
	//	モデルのパスとテクスチャのパスから合成パスを得る
	//	@param	modelPath	アプリケーションから見たpmdモデルのパス
	//	@param	texPath		PMDモデルから見たテクスチャのパス
	//	@return	texPath		アプリケーションから見たテクスチャのパス
	std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
		// ファイルのフォルダの区切りは\と/の二種類が使用される可能性があり
		// ともかく末尾の\か/を得られればいいので、双方のrfindをとり比較する
		// int型に代入しているのは見つからなかった場合はrfindがepos(-1→0xffffffff)を返すため
		int pathIndex1 = (int)modelPath.rfind('/');
		int pathIndex2 = (int)modelPath.rfind('\\');
		auto pathIndex = max(pathIndex1, pathIndex2);
		auto folderPath = modelPath.substr(0, (size_t)pathIndex + 1);
		return folderPath + texPath;
	}

	//	ファイル名から拡張子を取得する
	//	@param path	対象のパス文字列
	//	@return		拡張子
	std::string GetExtension(const std::string& path)
	{
		size_t idx = path.rfind('.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}

	//	ファイル名から拡張子を取得する
	//	@param path 対象のパス文字列
	//	@return 拡張子
	std::wstring GetExtension(const std::wstring& path) {
		size_t idx = path.rfind(L'.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}

	//	テクスチャのパスをセパレータ文字で分離する
	//	@param path		対象のパス文字列
	//	@param splitter	区切り文字
	//	@return			分離前後の文字列ペア
	std::pair<string, string> SplitFileName(const std::string& path, const char splitter = '*')
	{
		size_t idx = path.find(splitter);
		pair<string, string> ret;
		ret.first = path.substr(0, idx);
		ret.second = path.substr(idx + 1, path.length() - idx - 1);
		return ret;
	}

	// std::string（マルチバイト文字列）からstd::wstring（ワイド文字列）を得る
	// @param str マルチバイト文字列
	// @return 変換されたワイド文字列
	std::wstring GetWideStringFromString(const std::string& str)
	{
		// 呼び出し１回目（文字列数を得る）
		auto num1 = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			nullptr,
			0);

		std::wstring wstr;	// stringのwchar_t版
		wstr.resize(num1);	// 得られた文字列数でリサイズ

		// 呼び出し２回目（確保済みのwstrに変換文字列をコピー）
		auto num2 = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			&wstr[0],
			num1);

		assert(num1 == num2); // 一応チェック
		return wstr;
	}

	// デバッグレイヤーを有効にする
	void EnableDebugLayer() {
		ComPtr<ID3D12Debug> debugLayer = nullptr;
		D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.ReleaseAndGetAddressOf()));
		debugLayer->EnableDebugLayer();
	}
}

Dx12Wrapper::Dx12Wrapper(HWND hwnd) {
	// デバッグレイヤーをオンに
	DxDebug EnableDebugLayer();

	auto& app = Application::Instance();
	_winSize = app.GetWindowSize();

	// DirectX12関連初期化
	if (FAILED(InitializeDXGIDevice())) {
		assert(0);
		return;
	}
	if (FAILED(InitializeCommand())) {
		assert(0);
		return;
	}
	if (FAILED(CreateSwapChain(hwnd))) {
		assert(0);
		return;
	}
	if (FAILED(CreateFinalRenderTargets())) {
		assert(0);
		return;
	}
	if (FAILED(CreateSceneView())) {
		assert(0);
		return;
	}

	// テクスチャローダー関連初期化
	CreateTextureLoaderTable();

	// 深度バッファ作成
	if (FAILED(CreateDepthStencilView())) {
		assert(0);
		return;
	}

	// フェンスの作成
	if (FAILED(_dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf())))) {
		assert(0);
		return;
	}

}

HRESULT Dx12Wrapper::CreateDepthStencilView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapchain->GetDesc1(&desc);
	// 深度バッファ作成
	// 深度バッファの仕様
	// auto depthResDesc = CD3DX12RESOURCE_DESC::Tex2D(DXGI_FORAMT_D32_FLOAT,
	//	desc.Width, desc.Height,
	//	1,0,1,0,
	//	D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;		// 2次元のテクスチャデータ
	depthResDesc.Width				= desc.Width;								// 幅と高さはレンダーターゲットと同じ
	depthResDesc.Height				= desc.Height;								// 上と同じ
	depthResDesc.DepthOrArraySize	= 1;										// テクスチャ配列でもないし3Dテクスチャでもない
	depthResDesc.Format				= DXGI_FORMAT_D32_FLOAT;					// 深度値書き込み用フォーマット
	depthResDesc.SampleDesc.Count	= 1;										// サンプルは1ピクセル当たり1つ
	depthResDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// このバッファは深度ステンシルとして使用
	depthResDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResDesc.MipLevels			= 1;
	depthResDesc.Alignment			= 0;

	//デプス用ヒーププロパティ
	// auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type					= D3D12_HEAP_TYPE_DEFAULT;	// DEFAULTだから後はUNKNOWNでよし
	depthHeapProp.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;

	// クリア値の設定
	// CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
	D3D12_CLEAR_VALUE depthClearValue	= {};
	depthClearValue.DepthStencil.Depth = 1.0f;						// 深さ１（最大値）でクリア
	depthClearValue.Format				= DXGI_FORMAT_D32_FLOAT;	// 32bit深度値としてクリア

	// 深度バッファリソースの作成
	result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// デプス書き込みに使用
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		return result;
	}

	// 深度のためのデスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};// 深度に使用
	dsvHeapDesc.NumDescriptors	= 1;								// 深度ビュー１つのみ
	dsvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	// デプスステンシルビューとして利用
	dsvHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = _dev->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		return result;
	}

	// 深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format			= DXGI_FORMAT_D32_FLOAT;			// デプス値に32bit使用
	dsvDesc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
	dsvDesc.Flags			= D3D12_DSV_FLAG_NONE;				// フラグは特になし
	_dev->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

Dx12Wrapper::~Dx12Wrapper() 
{
	for (auto p : _backBuffers)
	{
		safeRelease(p);
	}
}

ComPtr<ID3D12Resource> Dx12Wrapper::GetTextureByPath(const char* texpath) {
	auto it = _textureTable.find(texpath);
	if (it != _textureTable.end()) {
		// テーブル内にあったらロードするのではなくてマップ内のリソースを返す
		return _textureTable[texpath];
	}
	else {
		return CreateTextureFromFile(texpath);
	}
}

// テクスチャローダテーブルの作成
void Dx12Wrapper::CreateTextureLoaderTable()
{
	_loadLambdaTable["sph"]
		= _loadLambdaTable["spa"]
		= _loadLambdaTable["bmp"]
		= _loadLambdaTable["png"]
		= _loadLambdaTable["jpg"]
		= [](const std::wstring& path, DirectX::TexMetadata* meta, DirectX::ScratchImage& img)->HRESULT
	{
		return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
	};
	
	_loadLambdaTable["tga"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
	{
		return LoadFromTGAFile(path.c_str(), meta, img);
	};
	
	_loadLambdaTable["dds"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
	{
		return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, img);
	};
}

// テクスチャの読み込み
ComPtr<ID3D12Resource> Dx12Wrapper::CreateTextureFromFile(const char* texpath) {
	string texPath = texpath;

	// WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	// テクスチャのファイルパス
	auto wtexpath = GetWideStringFromString(texPath);

	// 拡張子を取得
	auto ext = GetExtension(texPath);

	auto result = _loadLambdaTable[ext](wtexpath, &metadata, scratchImg);

	if (FAILED(result))
	{
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);// 生データ抽出

	// WriteToSubresourceで転送する用のヒープ設定
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	{/*
		D3D12_HEAP_PROPERTIES texHeapProp = {};
		texHeapProp.Type					= D3D12_HEAP_TYPE_CUSTOM;
		texHeapProp.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		texHeapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_L0;
		texHeapProp.CreationNodeMask		= 0;	// 単一アダプタのため0
		texHeapProp.VisibleNodeMask			= 0;	// 単一アダプタのため0
	*/}

	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		(UINT64)metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);
	{/*
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format				= metadata.format;
		resDesc.Width				= (UINT64)metadata.width;		// 幅
		resDesc.Height				= (UINT)metadata.height;		// 高さ
		resDesc.DepthOrArraySize	= (UINT16)metadata.arraySize;
		resDesc.SampleDesc.Count	= 1;					// 通常テクスチャなのでアンチエイリアシングしない
		resDesc.SampleDesc.Quality	= 0;					// クオリティは最低
		resDesc.MipLevels			= (UINT16)metadata.mipLevels;
		resDesc.Dimension			= static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
		resDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;	// レイアウトは決定しない
		resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;		// 特にフラグなし
	*/}

	// バッファー作成
	ComPtr<ID3D12Resource> texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(texbuff.ReleaseAndGetAddressOf())
	);
	DxDebug texbuff->SetName(L"texbuff");
	if (FAILED(result))	{
		return nullptr;
	}

	result = texbuff->WriteToSubresource(
		0,
		nullptr,				// 全領域へコピー
		img->pixels,			// 元データアドレス
		(UINT)img->rowPitch,	// １ラインサイズ
		(UINT)img->slicePitch	// 全サイズ
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	return texbuff;
}

// DirectXGIデバイスの初期化
HRESULT Dx12Wrapper::InitializeDXGIDevice()
{
	UINT flagsDXGI = 0;
	flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
	auto result = CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));

	// DirectX12初期化
	// フィーチャレベル列挙
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	if (FAILED(result)) {
		return result;
	}

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

	result = S_FALSE;

	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (SUCCEEDED(D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())))) {
			featureLevel = l;
			result = S_OK;
			break;
		}
	}

	for (auto adpt : adapters) {
		safeRelease(adpt);
	}

	return result;
}

// スワップチェイン生成関数
HRESULT Dx12Wrapper::CreateSwapChain(const HWND& hwnd)
{
	RECT rc = {};
	::GetWindowRect(hwnd, &rc);

	// スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width					= _winSize.cx;
	swapchainDesc.Height				= _winSize.cy;
	swapchainDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo				= false;
	swapchainDesc.SampleDesc.Count		= 1;
	swapchainDesc.SampleDesc.Quality	= 0;
	swapchainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount			= 2;
	swapchainDesc.Scaling				= DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode				= DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	auto result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());

	assert(SUCCEEDED(result));
	return result;
}

// コマンド（Allocator/List/Queue）の作成
HRESULT Dx12Wrapper::InitializeCommand()
{
	// コマンドアロケータの作成
	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// コマンドリストの作成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// コマンドキューの作成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;			// タイムアウトなし
	cmdQueueDesc.NodeMask	= 0;
	cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;		// プライオリティ特に指定なし
	cmdQueueDesc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;			// ここはコマンドリストと合わせる
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));	// コマンドキュー

	assert(SUCCEEDED(result));
	return result;
}

// ビュープロジェクション用ビューの生成
HRESULT Dx12Wrapper::CreateSceneView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapchain->GetDesc1(&desc);

	// 定数バッファ作成
	auto heapPropTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto matirxCBufferDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	result = _dev->CreateCommittedResource(
		&heapPropTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&matirxCBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneConstBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	_mappedSceneData = nullptr;// マップ先を示すポインタ
	result = _sceneConstBuff->Map(0, nullptr, (void**)&_mappedSceneData);//マップ

	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	//auto worldMat = XMMatrixIdentity();
	auto viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);
	// 定数バッファ作成
	auto projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,	//画角は45°
		static_cast<float>(desc.Width) / static_cast<float>(desc.Height),	//アス比
		0.1f,
		1000.0f
	);

	// 行列の内容をコピー
	//_mappedSceneData->world = worldMat;
	_mappedSceneData->view	= viewMat;
	_mappedSceneData->proj	= projMat;
	_mappedSceneData->eye	= eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// シェーダ―から見える
	descHeapDesc.NodeMask		= 0;											// マスクは0
	descHeapDesc.NumDescriptors	= 1;											// CBV１つ
	descHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));// 生成

	// ディスクリプタの先頭アドレスを取得
	auto heapHandle = _sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation	= _sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes		= (UINT)_sceneConstBuff->GetDesc().Width;

	// 定数バッファビューの作成
	_dev->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;
}

// D3Dデバイスが保持しているオブジェクト情報を出力
void Dx12Wrapper::ReportD3DObject()
{
#ifdef  _DEBUG
	if (_dev == nullptr)
		return;
	DxDebug DebugOutputFormatString("-----------------------ReportD3DObject-----------------------\n");
	ID3D12DebugDevice* debugDevice = nullptr;
	auto result = _dev->QueryInterface(&debugDevice);
	if (SUCCEEDED(result))
	{
		debugDevice->ReportLiveDeviceObjects(
			D3D12_RLDO_DETAIL
			| D3D12_RLDO_IGNORE_INTERNAL 
			//| D3D12_RLDO_SUMMARY
		);
		debugDevice->Release();
	}
	DxDebug DebugOutputFormatString("-------------------------------------------------------------\n");
#endif //  _DEBUG
}

// レンダーターゲットの作成
HRESULT Dx12Wrapper::CreateFinalRenderTargets() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapchain->GetDesc1(&desc);

	// デスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビューなので当然RTV
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = 2;								// 表裏の２つ
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし

	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_rtvHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		SUCCEEDED(result);
		return result;
	}

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	_backBuffers.resize(swcDesc.BufferCount);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	// SRGBレンダーターゲットビュー設定
	// これやると色味がよくなるが、バックバッファとの
	// フォーマットの食い違いによりDebugLayerにエラーが出力される
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension	= D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < (int)swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
		rtvDesc.Format = _backBuffers[i]->GetDesc().Format;
		_dev->CreateRenderTargetView(_backBuffers[i], &rtvDesc, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DxDebug _backBuffers[i]->SetName(L"backBuffer");
	}

	_viewport.reset(new CD3DX12_VIEWPORT(_backBuffers[0]));
	{/*
		D3D12_VIEWPORT viewport = {};
		viewport.Width		= window_width;		// 出力先の幅（ピクセル数）
		viewport.Height		= window_height;	// 出力先の高さ（ピクセル数）
		viewport.TopLeftX	= 0;				// 出力先の左上座標X
		viewport.TopLeftY	= 0;				// 出力先の左上座標Y
		viewport.MaxDepth	= 1.0f;				// 深度最大値
		viewport.MinDepth	= 0.0f;				// 深度最小値
	*/}

	_scissorrect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));
	//_scissorrect = CD3DX12_RECT(0, 0, window_width, window_height);
	{/*
		D3D12_RECT scissorrect = {};
		scissorrect.top		= 0;								// 切り抜き上座標
		scissorrect.left	= 0;								// 切り抜き左座標
		scissorrect.right	= scissorrect.left + window_width;	// 切り抜き右座標
		scissorrect.bottom	= scissorrect.top + window_height;	// 切り抜き下座標
	*/}

	return result;
}


ComPtr<ID3D12Device> Dx12Wrapper::Device() {
	return _dev;
}

ComPtr<ID3D12GraphicsCommandList> Dx12Wrapper::CommandList() {
	return _cmdList;
}

void Dx12Wrapper::Update()
{

}

void Dx12Wrapper::BeginDraw()
{
	// DirectX処理
	// バックバッファのインデックスを取得
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	auto renderTargetBarrierDesc =
		CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	_cmdList->ResourceBarrier(1, &renderTargetBarrierDesc);
	{/*
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource	= _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &BarrierDesc);
	*/}

	// レンダーターゲットを指定
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<unsigned long long>(bbIdx) * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 深度を指定
	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 画面クリア
	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	// ビューポート、シザー矩形のセット
	_cmdList->RSSetViewports(1, _viewport.get());
	_cmdList->RSSetScissorRects(1, _scissorrect.get());
}

void Dx12Wrapper::SetScene() {
	// 現在のシーン（ビュープロジェクション）をセット
	ID3D12DescriptorHeap* sceneheaps[] = { _sceneDescHeap.Get() };
	_cmdList->SetDescriptorHeaps(1, sceneheaps);
	_cmdList->SetGraphicsRootDescriptorTable(0, _sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void Dx12Wrapper::EndDraw()
{
	// DirectX処理
	// バックバッファのインデックスを取得
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	auto presentBarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_cmdList->ResourceBarrier(1, &presentBarrierDesc);
	{/*
		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);
	*/}

	//命令のクローズ
	_cmdList->Close();

	// コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	// 待ち
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() < _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		if (event)
		{
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	}

	_cmdAllocator->Reset();							// キューをクリア
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);	// 再びコマンドリストをためる準備
}

ComPtr<IDXGISwapChain4> Dx12Wrapper::Swapchain() {
	return _swapchain;
}
