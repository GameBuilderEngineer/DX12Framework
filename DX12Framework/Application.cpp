//コンスタントバッファで行列を転送

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
}

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
}

HRESULT Application::CreateBassicGraphicsPipeline()
{
	return S_OK;
}

HRESULT Application::CreateRootSignature()
{
	return S_OK;
}

void Application::Run()
{

}

void Application::CreateTextureLoaderTable()
{

}

HRESULT Application::CreateDepthStencilView()
{
	return S_OK;
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