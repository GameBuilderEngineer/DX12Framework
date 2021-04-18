﻿//コンスタントバッファで行列を転送

#include "Application.h"

#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

template <class T>
void safeRelease(T* p)
{
	if (p != nullptr)
		p->Release();
	p = nullptr;
}


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
	if (msg == WM_DESTROY) {	// ウィンドウ破棄時呼び出し
		PostQuitMessage(0);		// OSに対して終了メッセージ
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// ウィンドウ定数
const unsigned int window_width = 1280;
const unsigned int window_height = 720;

/// モデルのパスとテクスチャのパスから合成パスを得る
///	@param	modelPath	アプリケーションから見たpmdモデルのパス
///	@param	texPath		PMDモデルから見たテクスチャのパス
///	@return	texPath		アプリケーションから見たテクスチャのパス
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

/// ファイル名から拡張子を取得する
/// @param path	対象のパス文字列
/// @return		拡張子
std::string GetExtension(const std::string& path)
{
	size_t idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

// ファイル名から拡張子を取得する
// @param path 対象のパス文字列
// @return 拡張子
std::wstring GetExtension(const std::wstring& path) {
	size_t idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}


// テクスチャのパスをセパレータ文字で分離する
// @param path		対象のパス文字列
// @param splitter	区切り文字
// @return 分離前後の文字列ペア
std::pair<string, string> SplitFileName(const std::string& path, const char splitter = '*')
{
	size_t idx = path.find(splitter);
	pair<string, string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx+1, path.length() - idx - 1);
	return ret;
}

/// std::string（マルチバイト文字列）からstd::wstring（ワイド文字列）を得る
/// @param str マルチバイト文字列
/// @return 変換されたワイド文字列
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

// デフォルトグラデーションテクスチャ
ComPtr<ID3D12Resource> Application::CreateGrayGradationTexture()
{
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 256);
	{/*同上
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		resDesc.Width				= 4;									// 幅
		resDesc.Height				= 256;									// 高さ
		resDesc.DepthOrArraySize	= 1;
		resDesc.SampleDesc.Count	= 1;
		resDesc.SampleDesc.Quality	= 0;
		resDesc.MipLevels			= 1;
		resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;			// レイアウトについては決定しない
		resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;				// とくにフラグなし
	*/}

	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	{/*
		D3D12_HEAP_PROPERTIES texHeapProp = {};
		texHeapProp.Type					= D3D12_HEAP_TYPE_CUSTOM;				// 特殊な設定なのでdefaultでもuploadでもない
		texHeapProp.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	// ライトバックで
		texHeapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_L0;					// 転送がL0つまりCPU側から直で
		texHeapProp.CreationNodeMask		= 0;									// 単一アダプタのため0
		texHeapProp.VisibleNodeMask			= 0;									// 単一アダプタのため0
	*/}

	ComPtr<ID3D12Resource> gradBuff = nullptr;
	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(gradBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	// 上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (c << 0xff) | (c << 16) | (c << 8) | c;
		std::fill(it, it + 4, col);
		--c;
	}

	result = gradBuff->WriteToSubresource(
		0, 
		nullptr, 
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * (UINT)data.size()
	);

	return gradBuff;
}

// 白テクスチャの作成
ComPtr<ID3D12Resource> Application::CreateWhiteTexture()
{
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	{/*
		D3D12_HEAP_PROPERTIES texHeapProp = {};
		texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		// texHeapProp.CreationNodeMask = 0;
		texHeapProp.VisibleNodeMask = 0;
	*/}

	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 4);
	{/*
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resDesc.Width = 4;	// 幅
		resDesc.Height = 4;	// 高さ
		resDesc.DepthOrArraySize = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.MipLevels = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	*/}

	ComPtr<ID3D12Resource> whiteBuff = nullptr;

	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(whiteBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);	// 全部255で埋める

	// データ転送
	result = whiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		(UINT)data.size()
	);

	return whiteBuff;
}

// 黒テクスチャの作成
ComPtr<ID3D12Resource> Application::CreateBlackTexture()
{
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	{/*
		D3D12_HEAP_PROPERTIES texHeapProp = {};
		texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		// texHeapProp.CreationNodeMask = 0;
		texHeapProp.VisibleNodeMask = 0;
	*/}

	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 4);
	{/*
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resDesc.Width = 4;	// 幅
		resDesc.Height = 4;	// 高さ
		resDesc.DepthOrArraySize = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.MipLevels = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	*/}

	ComPtr<ID3D12Resource> blackBuff = nullptr;

	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(blackBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);	// 全部0で埋める

	// データ転送
	result = blackBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		(UINT)data.size()
	);

	return blackBuff;
}

// テクスチャの読み込み
ComPtr<ID3D12Resource> Application::LoadTextureFromFile(std::string& texPath) {
	auto it = _resourceTable.find(texPath);
	if (it != _resourceTable.end())
	{
		// テーブル内にあったらロードするのではなくマップ内の
		// リソースを返す
		return _resourceTable[texPath];
	}
	
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

	if (FAILED(result))
	{
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

	_resourceTable[texPath] = texbuff;

	return texbuff;
}

// デバッグレイヤーを有効にする
void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

// スワップチェイン生成関数
HRESULT Application::CreateSwapChain(const HWND& hwnd, ComPtr<IDXGIFactory6> dxgiFactory)
{
	// スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width					= window_width;
	swapchainDesc.Height				= window_height;
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
	
	return _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());
}

// ゲームウィンドウの生成
void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	
	//ウィンドウクラス生成＆登録
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc		= (WNDPROC)WindowProcedure;	// コールバック関数の指定
	windowClass.lpszClassName	= _T("DirectXTex");			// アプリケーションクラス名
	windowClass.hInstance		= GetModuleHandle(0);		// ハンドルの取得
	RegisterClassEx(&windowClass);							// アプリケーションクラス
	
	RECT wrc = { 0,0,window_width, window_height };			// ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);		// ウィンドウのサイズを関数を使って補正する
	
	// ウィンドウオブジェクトの生成
	hwnd = CreateWindow(
		windowClass.lpszClassName,	// クラス名指定
		_T("DX12テスト"),			// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,		// タイトルバーと境界線
		CW_USEDEFAULT,				// 表示X座標はOS
		CW_USEDEFAULT,				// 表示Y座標はOS
		wrc.right - wrc.left,		// ウィンドウ幅
		wrc.bottom - wrc.top,		// ウィンドウ高
		nullptr,					// 親ウィンドウハンドル
		nullptr,					// メニューハンドル
		windowClass.hInstance,		// 呼び出しアプリケーションハンドル
		nullptr);					// 追加パラメータ
}

// DirectXGIデバイスの初期化
HRESULT Application::InitializeDXGIDevice()
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
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK) {
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

// コマンド（Allocator/List/Queue）の作成
HRESULT Application::InitializeCommand()
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
	
	if (FAILED(result)) {
		assert(0);
	}
	return result;
}

// レンダーターゲットの作成
HRESULT Application::CreateFinalRenderTarget(ComPtr<ID3D12DescriptorHeap>& rtvHeaps, vector<ID3D12Resource*>& backBuffers)
{
	// デスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビューなので当然RTV
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = 2;								// 表裏の２つ
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし

	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	backBuffers.resize(swcDesc.BufferCount);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	// SRGBレンダーターゲットビュー設定
	// これやると色味がよくなるが、バックバッファとの
	// フォーマットの食い違いによりDebugLayerにエラーが出力される
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension	= D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < (int)swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		rtvDesc.Format = backBuffers[i]->GetDesc().Format;
		_dev->CreateRenderTargetView(backBuffers[i], &rtvDesc, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	_viewport = CD3DX12_VIEWPORT(backBuffers[0]);
	{/*
		D3D12_VIEWPORT viewport = {};
		viewport.Width		= window_width;		// 出力先の幅（ピクセル数）
		viewport.Height		= window_height;	// 出力先の高さ（ピクセル数）
		viewport.TopLeftX	= 0;				// 出力先の左上座標X
		viewport.TopLeftY	= 0;				// 出力先の左上座標Y
		viewport.MaxDepth	= 1.0f;				// 深度最大値
		viewport.MinDepth	= 0.0f;				// 深度最小値
	*/}

	_scissorrect = CD3DX12_RECT(0, 0, window_width, window_height);
	{/*
		D3D12_RECT scissorrect = {};
		scissorrect.top		= 0;								// 切り抜き上座標
		scissorrect.left	= 0;								// 切り抜き左座標
		scissorrect.right	= scissorrect.left + window_width;	// 切り抜き右座標
		scissorrect.bottom	= scissorrect.top + window_height;	// 切り抜き下座標
	*/}
	return result;
}

// パイプライン初期化
HRESULT Application::CreateBasicGraphicsPipeline()
{
	// シェーダ―
	ComPtr<ID3DBlob> _vsBlob = nullptr;
	ComPtr<ID3DBlob> _psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	// 頂点シェーダーの読み込み
	auto result = D3DCompileFromFile(
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
			errorDebug(errorBlob.Get());
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
			errorDebug(errorBlob.Get());
		}
		exit(1);
	}

	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",	0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",		0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",	0,DXGI_FORMAT_R32G32_FLOAT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"BONE_NO",		0,DXGI_FORMAT_R16G16_UINT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"WEIGHT",		0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		//{"EDGE_FLG",	0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature		= _rootsignature.Get();
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(_vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(_psBlob.Get());
	{/*
		gpipeline.VS.pShaderBytecode	= _vsBlob->GetBufferPointer();
		gpipeline.VS.BytecodeLength		= _vsBlob->GetBufferSize();
		gpipeline.PS.pShaderBytecode	= _psBlob->GetBufferPointer();
		gpipeline.PS.BytecodeLength		= _psBlob->GetBufferSize();
	*/}

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 貼るシェーダ、ドメインシェーダー、ジオメトリシェーダーは設定しない
	gpipeline.HS.BytecodeLength		= 0;
	gpipeline.HS.pShaderBytecode	= nullptr;
	gpipeline.DS.BytecodeLength		= 0;
	gpipeline.DS.pShaderBytecode	= nullptr;
	gpipeline.GS.BytecodeLength		= 0;
	gpipeline.GS.pShaderBytecode	= nullptr;

	// ラスタライザ(RS)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
	{/*
		gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	// 中身を塗りつぶす
		gpipeline.RasterizerState.FrontCounterClockwise = false;
		gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		gpipeline.RasterizerState.DepthClipEnable = true;			// 深度方向のクリッピングを有効にする
		gpipeline.RasterizerState.MultisampleEnable = false;		// アンチエイリアシングを使用しない
		gpipeline.RasterizerState.AntialiasedLineEnable = false;
		gpipeline.RasterizerState.ForcedSampleCount = 0;
		gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 三角形で構成
	*/}

	// OutputMarger部分
	// レンダーターゲット
	gpipeline.NumRenderTargets = 1;// このターゲット数と設定するファーマっと数は一致させる
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	// 0~1に正規化されたRGBA

	// 深度ステンシル
	gpipeline.DepthStencilState.DepthEnable		= true;
	gpipeline.DepthStencilState.StencilEnable	= false;
	gpipeline.DSVFormat							= DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;

	//ブレンド設定
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	{/*
		gpipeline.BlendState.AlphaToCoverageEnable		= false;
		gpipeline.BlendState.IndependentBlendEnable		= false;
		gpipeline.BlendState.RenderTarget->BlendEnable	= true;
		gpipeline.BlendState.RenderTarget->SrcBlend		= D3D12_BLEND_SRC_ALPHA;
		gpipeline.BlendState.RenderTarget->DestBlend	= D3D12_BLEND_INV_SRC_ALPHA;
		gpipeline.BlendState.RenderTarget->BlendOp		= D3D12_BLEND_OP_ADD;
	*/}
	
	gpipeline.NodeMask				= 0;
	gpipeline.SampleDesc.Count		= 1;	// サンプリングは１ピクセルにつき１
	gpipeline.SampleDesc.Quality	= 0;
	gpipeline.SampleMask			= 0xffffffff;// 全部対象
	gpipeline.Flags					= D3D12_PIPELINE_STATE_FLAG_NONE;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	// ひとまず加算や乗算やαブレンディングは使用しない
	renderTargetBlendDesc.BlendEnable			= false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable			= false;

	gpipeline.BlendState.RenderTarget[0]		= renderTargetBlendDesc;

	gpipeline.InputLayout.pInputElementDescs	= inputLayout;		// レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements			= _countof(inputLayout);	// レイアウト配列数

	gpipeline.IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	// ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で構成
	gpipeline.NumRenderTargets					= 1;							//今は１つのみ
	gpipeline.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;	//0～1に正規化されたRGBA

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
	}
	return result;
}

// ルートシグネチャ初期化
HRESULT Application::CreateRootSignature()
{
	// レンジ
	CD3DX12_DESCRIPTOR_RANGE descTblRange[3] = {};	// テクスチャと定数の２つ
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // 定数[b0]（座標変換用）
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // 定数[b1]（マテリアル用）
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); // テクスチャ4つ
	{/*
		// 定数用ディスクリプタレンジ
		D3D12_DESCRIPTOR_RANGE descTblRange[3] = {};	// テクスチャと定数の２つ
		
		// 定数一つ目（座標変換用）
		descTblRange[0].NumDescriptors						= 1;	//	定数ひとつ
		descTblRange[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
		descTblRange[0].BaseShaderRegister					= 0;	// 0番スロットから
		descTblRange[0].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		
		// 定数二つ目（マテリアル用）
		descTblRange[1].NumDescriptors						= 1;	//	ディスクリプタヒープは複数だが一度に使うのは一つ
		descTblRange[1].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
		descTblRange[1].BaseShaderRegister					= 1;	// 1番スロットから
		descTblRange[1].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// テクスチャ一つ目（↑のマテリアルとペア）
		descTblRange[2].NumDescriptors						= 4;	//	テクスチャ4つ(基本とsphとspaとtoon)
		descTblRange[2].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// 種別はテクスチャ
		descTblRange[2].BaseShaderRegister					= 0;	// 0番スロットから
		descTblRange[2].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	*/}


	// ルートパラメーター
	CD3DX12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);	// 座標変換
	rootparam[1].InitAsDescriptorTable(2, &descTblRange[1]);	// マテリアル周り
	{/*
		D3D12_ROOT_PARAMETER rootparam[2] = {};
		rootparam[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[0].DescriptorTable.pDescriptorRanges		= &descTblRange[0];	// ディスクリプタレンジのアドレス
		rootparam[0].DescriptorTable.NumDescriptorRanges	= 1;	// ディスクリプタレンジ数
		rootparam[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY_ALL;	// 全てのシェーダーから見える

		rootparam[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[1].DescriptorTable.pDescriptorRanges		= &descTblRange[1];	// ディスクリプタレンジのアドレス
		rootparam[1].DescriptorTable.NumDescriptorRanges	= 2;	// ディスクリプタレンジ数
		rootparam[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;	// ピクセルシェーダーから見える
	*/}

	// サンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(
		1,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	{/*
		D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
		samplerDesc[0].AddressU				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 横繰り返し
		samplerDesc[0].AddressV				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 縦繰り返し
		samplerDesc[0].AddressW				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 奥繰り返し
		samplerDesc[0].BorderColor			= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	// ボーダーの時は黒
		samplerDesc[0].Filter				= D3D12_FILTER_MIN_MAG_MIP_POINT;				// 補間しない（ニアレストネイバー）
		samplerDesc[0].MaxLOD				= D3D12_FLOAT32_MAX;							// ミップマップ最大値
		samplerDesc[0].MinLOD				= 0.0f;											// ミップマップ最小値
		samplerDesc[0].ComparisonFunc		= D3D12_COMPARISON_FUNC_NEVER;					// オーバーサンプリングの際リサンプリングしない
		samplerDesc[0].ShaderVisibility		= D3D12_SHADER_VISIBILITY_PIXEL;				// ピクセルシェーダーからのみ可視
	
		samplerDesc[1] = samplerDesc[0];	// 変更点以外をコピー
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
		samplerDesc[1].ShaderRegister = 1;							// シェーダ―スロット番号を忘れないように
	*/}

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags				= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters		= rootparam;	// ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters		= 2;			// ルートパラメータ数
	rootSignatureDesc.pStaticSamplers	= samplerDescs;
	rootSignatureDesc.NumStaticSamplers = 2;
	{/* コード視認性は低下するので、微妙かもしれない・・・
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(
			2,
			rootparam,
			2,
			samplerDesc,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	*/}

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	errorDebug(errorBlob.Get());
	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(_rootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	return result;
}

void Application::Run()
{

}

// テクスチャローダテーブルの作成
void Application::CreateTextureLoaderTable()
{
	_loadLambdaTable["sph"]
		= _loadLambdaTable["spa"]
		= _loadLambdaTable["bmp"]
		= _loadLambdaTable["png"]
		= _loadLambdaTable["jpg"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
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
		return LoadFromDDSFile(path.c_str(),DDS_FLAGS_NONE, meta, img);
	};
}

// デプスステンシルビューの生成
HRESULT Application::CreateDepthStencilView()
{
	// 深度バッファ作成
	// 深度バッファの仕様
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;		// 2次元のテクスチャデータ
	depthResDesc.Width				= window_width;								// 幅と高さはレンダーターゲットと同じ
	depthResDesc.Height				= window_height;							// 上と同じ
	depthResDesc.DepthOrArraySize	= 1;										// テクスチャ配列でもないし3Dテクスチャでもない
	depthResDesc.Format				= DXGI_FORMAT_D32_FLOAT;					// 深度値書き込み用フォーマット
	depthResDesc.SampleDesc.Count	= 1;										// サンプルは1ピクセル当たり1つ
	depthResDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// このバッファは深度ステンシルとして使用
	depthResDesc.MipLevels			= 1;
	depthResDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResDesc.Alignment			= 0;

	//デプス用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type					= D3D12_HEAP_TYPE_DEFAULT;				// DEFAULTだから後はUNKNOWNでよし
	depthHeapProp.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;

	// クリア値の設定
	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
	{/*
		D3D12_CLEAR_VALUE _depthClearValue	= {};
		_depthClearValue.DepthStencil.Depth = 1.0f;						// 深さ１（最大値）でクリア
		_depthClearValue.Format				= DXGI_FORMAT_D32_FLOAT;	// 32bit深度値としてクリア
	*/}

	// 色のクリア値(未使用)
	float clrColor[4] = { 1.0f,1.0f,1.0f,1.0f };
	CD3DX12_CLEAR_VALUE rtClearValue(DXGI_FORMAT_R8G8B8A8_UINT, clrColor);

	// 深度バッファリソースの作成
	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	auto result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// デプス書き込みに使用
		&depthClearValue,
		IID_PPV_ARGS(depthBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		return result;
	}

	// 深度のためのデスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};		// 深度に使用
	dsvHeapDesc.NumDescriptors = 1;						// 深度ビュー１つのみ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビューとして利用
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		return result;
	}

	// 深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format			= DXGI_FORMAT_D32_FLOAT;			// デプス値に32bit使用
	dsvDesc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
	dsvDesc.Flags			= D3D12_DSV_FLAG_NONE;				// フラグは特になし
	_dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

// PMDファイルのロード
HRESULT Application::LoadPMDFile(const char* path)
{
	// PMDヘッダ構造体
	struct PMDHeader {
		float version;			// 例：00 00 80 3F == 1.00
		char model_name[20];	// モデル名
		char comment[256];		// モデルコメント
	};
	char signature[3];
	PMDHeader pmdheader = {};

	string strModelPath = path;

	auto fp = fopen(strModelPath.c_str(), "rb");
	if (fp == nullptr) {
		assert(0);
		return ERROR_FILE_NOT_FOUND;
	}
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);
	
	unsigned int vertNum;	// 頂点数
	fread(&vertNum, sizeof(vertNum), 1, fp);

#pragma pack(1)// ここから1バイトパッキング…アライメントは発生しない
	// PMDマテリアル構造体
	struct PMDMaterial {
		XMFLOAT3 diffuse;			// ディフューズ色
		float alpha;				// ディフューズα
		float specularity;			// スペキュラの強さ（乗算値）
		XMFLOAT3 specular;			// スペキュラ色
		XMFLOAT3 ambient;			// アンビエント色
		unsigned char toonIdx;		// トゥーン番号
		unsigned char edgeFlg;		// マテリアル毎の輪郭線フラグ
		// 注意：2バイトのパディング
		unsigned int indicesNum;	// このマテリアルが割り当たるインデックス数
		char texFilePath[20];		// テクスチャファイル名(プラスアルファ)
	};// 70バイトのはず…でもパディングが発生するため72バイト
#pragma pack()// 1バイトパッキング解除

	constexpr unsigned int pmdvertex_size = 38;	// 頂点１つあたりのサイズ
	unsigned int verticesSize = vertNum * pmdvertex_size;
	std::vector<unsigned char> vertices(verticesSize);	// バッファ確保
	fread(vertices.data(), vertices.size(), 1, fp);// 読み込み

	unsigned int indicesNum;// インデックス数
	fread(&indicesNum, sizeof(indicesNum), 1, fp);

	// 頂点バッファの作成
	// UPLOAD(確保は可能)
	auto vertHeapProp	= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());
	auto result = _dev->CreateCommittedResource(
		&vertHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_vertBuffer.ReleaseAndGetAddressOf()));

	// 作ったバッファに頂点データをコピー
	unsigned char* vertMap = nullptr;
	result = _vertBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertBuffer->Unmap(0, nullptr);
	
	// 頂点バッファビューの作成
	_vbView.BufferLocation	= _vertBuffer->GetGPUVirtualAddress();	// バッファの仮想アドレス
	_vbView.SizeInBytes		= (UINT)vertices.size();				// 全バイト数
	_vbView.StrideInBytes	= pmdvertex_size;						// 1頂点あたりのバイト数
	
	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	// インデックスバッファーの作成
	// 設定は、バッファのサイズ以外頂点バッファの設定を使いまわしてOK
	auto indexHeapProp		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto indexBufferDesc	= CD3DX12_RESOURCE_DESC::Buffer(indices.size()*sizeof(indices[0]));
	result = _dev->CreateCommittedResource(
		&indexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_idxBuffer.ReleaseAndGetAddressOf()));

	// 作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	_idxBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_idxBuffer->Unmap(0, nullptr);

	// インデックスバッファビューを作成
	_ibView.BufferLocation	= _idxBuffer->GetGPUVirtualAddress();
	_ibView.Format			= DXGI_FORMAT_R16_UINT;
	_ibView.SizeInBytes		= (UINT)indices.size()*sizeof(indices[0]);


	// マテリアル数を読み込み
	fread(&_materialNum, sizeof(_materialNum), 1, fp);

	// リソースサイズを設定
	_materials.resize(_materialNum);
	_textureResources.resize(_materialNum);
	_sphResources.resize(_materialNum);
	_spaResources.resize(_materialNum);
	_toonResources.resize(_materialNum);

	// PMDマテリアル情報の読み込み
	std::vector<PMDMaterial> pmdMaterials(_materialNum);
	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
	// コピー
	for (int i = 0; i < pmdMaterials.size(); ++i)
	{
		_materials[i].indicesNum			= pmdMaterials[i].indicesNum;
		_materials[i].material.diffuse		= pmdMaterials[i].diffuse;
		_materials[i].material.alpha		= pmdMaterials[i].alpha;
		_materials[i].material.specular		= pmdMaterials[i].specular;
		_materials[i].material.specularity	= pmdMaterials[i].specularity;
		_materials[i].material.ambient		= pmdMaterials[i].ambient;
		_materials[i].additional.toonIdx	= pmdMaterials[i].toonIdx;
	}
	for (int i = 0; i < pmdMaterials.size(); ++i)
	{
		// トゥーンリソースの読み込み
		string toonFilePath = "toon/";
	
		char toonFileName[16];
	
		sprintf(toonFileName, "toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
		toonFilePath += toonFileName;
		_toonResources[i] = LoadTextureFromFile(toonFilePath);
	
		if (strlen(pmdMaterials[i].texFilePath) == 0)
		{
			_textureResources[i] = nullptr;
			continue;
		}
	
		// モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		string texFileName = pmdMaterials[i].texFilePath;
		string sphFileName = "";
		string spaFileName = "";
		
		if (std::count(texFileName.begin(), texFileName.end(), '*') > 0) {//スプリッタがある
			auto namepair = SplitFileName(texFileName);
			if (GetExtension(namepair.first) == "sph")
			{
				sphFileName = namepair.first;
				texFileName = namepair.second;
			}
			else if (GetExtension(namepair.first) == "spa")
			{
				spaFileName = namepair.first;
				texFileName = namepair.second;
			}
			else
			{
				texFileName = namepair.first;
				if (GetExtension(namepair.second) == "sph")
				{
					sphFileName = namepair.second;
				}
				else if (GetExtension(namepair.second) == "spa")
				{
					spaFileName = namepair.second;
				}
			}
		}
		else {
			if (GetExtension(texFileName) == "sph")
			{
				sphFileName = texFileName;
				texFileName = "";
			}
			else if (GetExtension(texFileName) == "spa")
			{
				spaFileName = texFileName;
				texFileName = "";
			}
		}
		
		// モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		// テクスチャの読み込み
		if (texFileName != "") {
			auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
			_textureResources[i] = LoadTextureFromFile(texFilePath);
		}
		// 乗算スフィアマップの読み込み
		if (sphFileName != "") {
			auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
			_sphResources[i] = LoadTextureFromFile(sphFilePath);
		}
		// 加算スフィアマップの読み込み
		if (spaFileName != "") {
			auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
			_spaResources[i] = LoadTextureFromFile(spaFilePath);
		}
	}
	
	fclose(fp);
	return result;
}

// GPU側のマテリアルデータの作成
HRESULT Application::CreateMaterialData() {
	// マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto materialResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * _materialNum);// もったいない
	auto result = _dev->CreateCommittedResource(
		&materialHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&materialResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_materialBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = _materialBuffer->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	for (auto& m : _materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material; // データコピー
		mapMaterial += materialBuffSize; // 次のアラインメント位置まで進める
	}
	_materialBuffer->Unmap(0, nullptr);

	// マテリアル用ディスクリプタヒープとビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
	matDescHeapDesc.NumDescriptors	= _materialNum * 5;	// マテリアル数(定数1つ,テクスチャ3つ)を指定
	matDescHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDescHeapDesc.NodeMask		= 0;
	matDescHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;// デスクリプタヒープ種別
	result = _dev->CreateDescriptorHeap(
		&matDescHeapDesc, 
		IID_PPV_ARGS(_materialDescHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// マテリアル用ビューの作成
	matCBVDesc.BufferLocation	= _materialBuffer->GetGPUVirtualAddress();	// バッファ―アドレス
	matCBVDesc.SizeInBytes		= (UINT)materialBuffSize;					// マテリアルの256アライメントサイズ
	return result;
}

HRESULT Application::CreateSceneTransformView()
{
	return S_OK;
}

void Application::CreateMaterialAndTextureView()
{

}

bool Application::Init() {
	return false;
}

void Application::Terminate() {

}

Application::Application() {

}

Application::~Application() {

}

Application& Application::Instance() {
	static Application instance;
	return instance;
}