#include "Application.h"
#include <Windows.h>

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	auto& app = Application::Instance();

	if (!app.Init())
	{
		return -1;
	}

	return 0;
}




//HRESULT InitializeDXGIDevice() {
//}
//
//HRESULT InitializeCommand() {
//	// コマンドアロケータの作成
//	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
//	if (FAILED(result)) {
//		assert(0);
//		return result;
//	}
//
//	// コマンドリストの作成
//	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
//	if (FAILED(result)) {
//		assert(0);
//		return result;
//	}
//
//	// コマンドキューの作成
//	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
//	cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;			// タイムアウトなし
//	cmdQueueDesc.NodeMask	= 0;
//	cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;		// プライオリティ特に指定なし
//	cmdQueueDesc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;			// ここはコマンドリストと合わせる
//	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));	// コマンドキュー
//
//	if (FAILED(result)) {
//		assert(0);
//	}
//}
//
//HRESULT CreateFinalRenderTarget(ComPtr<ID3D12DescriptorHeap>& rtvHeaps, vector<ComPtr<ID3D12Resource>>& backBuffers) {
//
//	// デスクリプタヒープの作成
//	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
//	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビューなので当然RTV
//	heapDesc.NodeMask		= 0;
//	heapDesc.NumDescriptors = 2;								// 表裏の２つ
//	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし
//
//	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));
//	if (FAILED(result)) {
//		assert(0);
//		return result;
//	}
//
//	DXGI_SWAP_CHAIN_DESC swcDesc = {};
//	result = _swapchain->GetDesc(&swcDesc);
//	backBuffers.resize(swcDesc.BufferCount);
//
//	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
//
//	// SRGBレンダーターゲットビュー設定
//	// これやると色味がよくなるが、バックバッファとの
//	// フォーマットの食い違いによりDebugLayerにエラーが出力される
//	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
//	rtvDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//	rtvDesc.ViewDimension	= D3D12_RTV_DIMENSION_TEXTURE2D;
//
//	for (int i = 0; i < (int)swcDesc.BufferCount; ++i)
//	{
//		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(backBuffers[i].ReleaseAndGetAddressOf()));
//		rtvDesc.Format = backBuffers[i]->GetDesc().Format;
//		_dev->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, handle);
//		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	}
//}
//
//// D3Dデバイスが保持しているオブジェクト情報を出力
//void ReportD3DObject()
//{
//	if (_dev == nullptr)
//		return;
//	ID3D12DebugDevice* debugDevice = nullptr;
//	auto result = _dev->QueryInterface(&debugDevice);
//	if (SUCCEEDED(result))
//	{
//		debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
//		debugDevice->Release();
//	}
//}
//
//void releaseResource()
//{
//	// スマートポインタにより不要に
//	{/*
//		safeRelease(_dxgiFactory);
//		safeRelease(_cmdAllocator);
//		safeRelease(_cmdList);
//		safeRelease(_cmdQueue);
//		safeRelease(_swapchain);
//	*/}
//}
//
//#ifdef _DEBUG
//int main() {
//	auto errorDebug = [](ID3DBlob* errorBlob)
//	{
//		if (errorBlob == nullptr)return;
//		std::string errstr;
//		errstr.resize(errorBlob->GetBufferSize());
//		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
//		errstr += "\n";
//		OutputDebugStringA(errstr.c_str());
//	};
//
//#else
//#include<Windows.h>
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
//#endif
//	DebugOutputFormatString("Show window test.");
//
//	HWND hwnd;
//	WNDCLASSEX windowClass = {};
//	CreateGameWindow(hwnd, windowClass);
//
//#ifdef _DEBUG
//	// デバッグレイヤーをオンに
//	EnableDebugLayer();
//#endif
//
//	HRESULT result = InitializeDXGIDevice();
//
//	result = InitializeCommand();
//
//	result = CreateSwapChain(hwnd, _dxgiFactory);
//
//	std::vector<ComPtr<ID3D12Resource>> _backBuffers;
//	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
//
//	result = CreateFinalRenderTarget(rtvHeaps, _backBuffers);
//
//	loadLambdaTable["sph"]
//		= loadLambdaTable["spa"]
//		= loadLambdaTable["bmp"]
//		= loadLambdaTable["png"]
//		= loadLambdaTable["jpg"]
//		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
//	{
//		return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
//	};
//
//	loadLambdaTable["tga"]
//		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
//	{
//		return LoadFromTGAFile(path.c_str(), meta, img);
//	};
//
//	loadLambdaTable["dds"]
//		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT
//	{
//		return LoadFromDDSFile(path.c_str(),DDS_FLAGS_NONE, meta, img);
//	};
//
//	// 深度バッファ作成
//	// 深度バッファの仕様
//	D3D12_RESOURCE_DESC depthResDesc = {};
//	depthResDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2次元のテクスチャデータ
//	depthResDesc.Width				= window_width;	// 幅と高さはレンダーターゲットと同じ
//	depthResDesc.Height				= window_height;	// 上と同じ
//	depthResDesc.DepthOrArraySize	= 1;	// テクスチャ配列でもないし3Dテクスチャでもない
//	depthResDesc.Format				= DXGI_FORMAT_D32_FLOAT;	// 深度値書き込み用フォーマット
//	depthResDesc.SampleDesc.Count	= 1;	// サンプルは1ピクセル当たり1つ
//	depthResDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//このバッファは深度ステンシルとして使用
//	depthResDesc.MipLevels			= 1;
//	depthResDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	depthResDesc.Alignment			= 0;
//
//	//デプス用ヒーププロパティ
//	D3D12_HEAP_PROPERTIES depthHeapProp = {};
//	depthHeapProp.Type					= D3D12_HEAP_TYPE_DEFAULT;	// DEFAULTだから後はUNKNOWNでよし
//	depthHeapProp.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	depthHeapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
//
//	// クリア値の設定
//	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
//	{/*
//		D3D12_CLEAR_VALUE _depthClearValue	= {};
//		_depthClearValue.DepthStencil.Depth = 1.0f;						// 深さ１（最大値）でクリア
//		_depthClearValue.Format				= DXGI_FORMAT_D32_FLOAT;	// 32bit深度値としてクリア
//	*/}
//
//	// 色のクリア値(未使用)
//	float clrColor[4] = { 1.0f,1.0f,1.0f,1.0f };
//	CD3DX12_CLEAR_VALUE rtClearValue(DXGI_FORMAT_R8G8B8A8_UINT, clrColor);
//
//	// 深度バッファリソースの作成
//	ComPtr<ID3D12Resource> depthBuffer = nullptr;
//	result = _dev->CreateCommittedResource(
//		&depthHeapProp,
//		D3D12_HEAP_FLAG_NONE,
//		&depthResDesc,
//		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// デプス書き込みに使用
//		&depthClearValue,
//		IID_PPV_ARGS(depthBuffer.ReleaseAndGetAddressOf())
//	);
//
//	// 深度のためのデスクリプタヒープ作成
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};	// 深度に使用
//	dsvHeapDesc.NumDescriptors = 1;	// 深度ビュー１つのみ
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビューとして利用
//	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
//	result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf()));
//
//	// 深度ビュー作成
//	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
//	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	// デプス値に32bit使用
//	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
//	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;	// フラグは特になし
//	_dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
//
//	// フェンスの作成
//	ComPtr<ID3D12Fence> _fence = nullptr;
//	UINT64 _fenceVal = 0;
//	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
//
//	// ウィンドウ表示
//	ShowWindow(hwnd, SW_SHOW);
//
//	auto whiteTex	= CreateWhiteTexture();
//	auto blackTex	= CreateBlackTexture();
//	auto gradTex	= CreateGrayGradationTexture();
//
//	// PMDヘッダ構造体
//	struct PMDHeader {
//		float version;			// 例：00 00 80 3F == 1.00
//		char model_name[20];	// モデル名
//		char comment[256];		// モデルコメント
//	};
//	char signature[3];
//	PMDHeader pmdheader = {};
//	//string strModelPath = "Model/hibiki/hibiki.pmd";
//	//string strModelPath = "Model/satori/satori.pmd";
//	//string strModelPath = "Model/reimu/reimu.pmd";
//	//string strModelPath = "Model/巡音ルカ.pmd";
//	string strModelPath = "Model/初音ミク.pmd";
//	//string strModelPath = "Model/初音ミクVer2.pmd";
//	//string strModelPath = "Model/初音ミクmetal.pmd";
//	//string strModelPath = "Model/咲音メイコ.pmd";
//	//string strModelPath = "Model/ダミーボーン.pmd";//NG
//	//string strModelPath = "Model/鏡音リン.pmd";
//	//string strModelPath = "Model/鏡音リン_act2.pmd";
//	//string strModelPath = "Model/カイト.pmd";
//	//string strModelPath = "Model/MEIKO.pmd";
//	//string strModelPath = "Model/亞北ネル.pmd";
//	//string strModelPath = "Model/弱音ハク.pmd";
//	auto fp = fopen(strModelPath.c_str(), "rb");
//	fread(signature, sizeof(signature), 1, fp);
//	fread(&pmdheader, sizeof(pmdheader), 1, fp);
//
//	unsigned int vertNum;	// 頂点数
//	fread(&vertNum, sizeof(vertNum), 1, fp);
//
//#pragma pack(1)// ここから1バイトパッキング…アライメントは発生しない
//	// PMDマテリアル構造体
//	struct PMDMaterial {
//		XMFLOAT3 diffuse;			// ディフューズ色
//		float alpha;				// ディフューズα
//		float specularity;			// スペキュラの強さ（乗算値）
//		XMFLOAT3 specular;			// スペキュラ色
//		XMFLOAT3 ambient;			// アンビエント色
//		unsigned char toonIdx;		// トゥーン番号
//		unsigned char edgeFlg;		// マテリアル毎の輪郭線フラグ
//		// 注意：2バイトのパディング
//		unsigned int indicesNum;	// このマテリアルが割り当たるインデックス数
//		char texFilePath[20];		// テクスチャファイル名(プラスアルファ)
//	};// 70バイトのはず…でもパディングが発生するため72バイト
//#pragma pack()// 1バイトパッキング解除
//
//	// シェーダ―側に投げられるマテリアルデータ
//	struct MaterialForHlsl {
//		XMFLOAT3 diffuse;	// ディフューズ色
//		float alpha;		// ディフューズα
//		XMFLOAT3 specular;	// スペキュラ色
//		float specularity;	// スペキュラの強さ（乗算値）
//		XMFLOAT3 ambient;	// アンビエント色
//		MaterialForHlsl()
//		{
//			diffuse = {};
//			alpha = 0.0f;
//			specular = {};
//			specularity = 0.0f;
//			ambient = {};
//		}
//	};
//	// それ以外のマテリアルデータ
//	struct AdditionalMaterial {
//		std::string texPath;	// テクスチャファイルパス
//		int toonIdx;			// トゥーン番号
//		bool edgeFlg;			// マテリアル毎の輪郭線フラグ
//		AdditionalMaterial() {
//			texPath = {};
//			toonIdx = 0;
//			edgeFlg = 0;
//		}
//	};
//	// まとめたもの
//	struct Material {
//		unsigned int indicesNum;	// インデックス数
//		MaterialForHlsl material;
//		AdditionalMaterial additional;
//		Material(){
//			indicesNum = 0;
//			material = {};
//			additional = {};
//		}
//	};
//
//	constexpr unsigned int pmdvertex_size = 38;	// 頂点１つあたりのサイズ
//	unsigned int verticesSize = vertNum * pmdvertex_size;
//	std::vector<unsigned char> vertices(verticesSize);	// バッファ確保
//	fread(vertices.data(), vertices.size(), 1, fp);// 読み込み
//
//	unsigned int indicesNum;// インデックス数
//	fread(&indicesNum, sizeof(indicesNum), 1, fp);
//
//	// 頂点バッファの作成
//	// UPLOAD(確保は可能)
//	ComPtr<ID3D12Resource> vertBuff = nullptr;
//	auto vertHeapProp	= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	auto vertBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());
//	result = _dev->CreateCommittedResource(
//		&vertHeapProp,
//		D3D12_HEAP_FLAG_NONE,
//		&vertBufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf()));
//
//	// 作ったバッファに頂点データをコピー
//	unsigned char* vertMap = nullptr;
//	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
//	std::copy(vertices.begin(), vertices.end(), vertMap);
//	vertBuff->Unmap(0, nullptr);
//
//	// 頂点バッファビューの作成
//	D3D12_VERTEX_BUFFER_VIEW vbView = {};
//	vbView.BufferLocation	= vertBuff->GetGPUVirtualAddress();	// バッファの仮想アドレス
//	vbView.SizeInBytes		= (UINT)vertices.size();// 全バイト数
//	vbView.StrideInBytes	= pmdvertex_size;	// 1頂点あたりのバイト数
//
//	std::vector<unsigned short> indices(indicesNum);
//	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
//
//	// マテリアル数を読み込み
//	unsigned int materialNum;
//	fread(&materialNum, sizeof(materialNum), 1, fp);
//	std::vector<Material> materials(materialNum);
//
//	// リソース
//	vector<ComPtr<ID3D12Resource>> textureResources(materialNum);
//	vector<ComPtr<ID3D12Resource>> sphResources(materialNum);
//	vector<ComPtr<ID3D12Resource>> spaResources(materialNum);
//	vector<ComPtr<ID3D12Resource>> toonResources(materialNum);
//
//	// PMDマテリアル情報の読み込み
//	std::vector<PMDMaterial> pmdMaterials(materialNum);
//	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
//	// コピー
//	for (int i = 0; i < pmdMaterials.size(); ++i)
//	{
//		materials[i].indicesNum = pmdMaterials[i].indicesNum;
//		materials[i].material.diffuse = pmdMaterials[i].diffuse;
//		materials[i].material.alpha = pmdMaterials[i].alpha;
//		materials[i].material.specular = pmdMaterials[i].specular;
//		materials[i].material.specularity = pmdMaterials[i].specularity;
//		materials[i].material.ambient = pmdMaterials[i].ambient;
//		materials[i].additional.toonIdx = pmdMaterials[i].toonIdx;
//	}
//
//	for (int i = 0; i < pmdMaterials.size(); ++i)
//	{
//		// トゥーンリソースの読み込み
//		string toonFilePath = "toon/";
//
//		char toonFileName[16];
//
//		sprintf(toonFileName, "toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
//		toonFilePath += toonFileName;
//		toonResources[i] = LoadTextureFromFile(toonFilePath);
//
//		if (strlen(pmdMaterials[i].texFilePath) == 0)
//		{
//			textureResources[i] = nullptr;
//			continue;
//		}
//
//		// モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
//		string texFileName = pmdMaterials[i].texFilePath;
//		string sphFileName = "";
//		string spaFileName = "";
//		
//		if (std::count(texFileName.begin(), texFileName.end(), '*') > 0) {//スプリッタがある
//			auto namepair = SplitFileName(texFileName);
//			if (GetExtension(namepair.first) == "sph")
//			{
//				sphFileName = namepair.first;
//				texFileName = namepair.second;
//			}
//			else if (GetExtension(namepair.first) == "spa")
//			{
//				spaFileName = namepair.first;
//				texFileName = namepair.second;
//			}
//			else
//			{
//				texFileName = namepair.first;
//				if (GetExtension(namepair.second) == "sph")
//				{
//					sphFileName = namepair.second;
//				}
//				else if (GetExtension(namepair.second) == "spa")
//				{
//					spaFileName = namepair.second;
//				}
//			}
//		}
//		else {
//			if (GetExtension(texFileName) == "sph")
//			{
//				sphFileName = texFileName;
//				texFileName = "";
//			}
//			else if (GetExtension(texFileName) == "spa")
//			{
//				spaFileName = texFileName;
//				texFileName = "";
//			}
//		}
//		
//		// モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
//		// テクスチャの読み込み
//		if (texFileName != "") {
//			auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
//			textureResources[i] = LoadTextureFromFile(texFilePath);
//		}
//		// 乗算スフィアマップの読み込み
//		if (sphFileName != "") {
//			auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
//			sphResources[i] = LoadTextureFromFile(sphFilePath);
//		}
//		// 加算スフィアマップの読み込み
//		if (spaFileName != "") {
//			auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
//			spaResources[i] = LoadTextureFromFile(spaFilePath);
//		}
//	}
//
//	fclose(fp);
//
//	// インデックスバッファーの作成
//	ComPtr<ID3D12Resource> idxBuff = nullptr;
//	auto indexHeapProp		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	auto indexBufferDesc	= CD3DX12_RESOURCE_DESC::Buffer(indices.size()*sizeof(indices[0]));
//	// 設定は、バッファのサイズ以外頂点バッファの設定を使いまわしてOK
//	result = _dev->CreateCommittedResource(
//		&indexHeapProp,
//		D3D12_HEAP_FLAG_NONE,
//		&indexBufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&idxBuff));
//
//	// 作ったバッファにインデックスデータをコピー
//	unsigned short* mappedIdx = nullptr;
//	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
//	std::copy(indices.begin(), indices.end(), mappedIdx);
//	idxBuff->Unmap(0, nullptr);
//
//	// インデックスバッファビューを作成
//	D3D12_INDEX_BUFFER_VIEW ibView = {};
//	ibView.BufferLocation	= idxBuff->GetGPUVirtualAddress();
//	ibView.Format			= DXGI_FORMAT_R16_UINT;
//	ibView.SizeInBytes		= (UINT)indices.size()*sizeof(indices[0]);
//
//	// マテリアルバッファを作成
//	auto materialBuffSize = sizeof(MaterialForHlsl);
//	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
//	ComPtr<ID3D12Resource> materialBuff = nullptr;
//	auto materialHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	auto materialResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);// もったいない
//	result = _dev->CreateCommittedResource(
//		&materialHeapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&materialResourceDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(materialBuff.ReleaseAndGetAddressOf())
//	);
//
//	// マップマテリアルにコピー
//	char* mapMaterial = nullptr;
//	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);
//	for (auto& m : materials) {
//		*((MaterialForHlsl*)mapMaterial) = m.material;	// データコピー
//		mapMaterial += materialBuffSize;	// 次のアラインメント位置まで進める
//	}
//	materialBuff->Unmap(0, nullptr);
//
//	// マテリアル用ディスクリプタヒープとビューの作成
//	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;
//	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
//	matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	matDescHeapDesc.NodeMask = 0;
//	matDescHeapDesc.NumDescriptors = materialNum * 5;	// マテリアル数x4(定数,テクスチャ3つ)を指定
//	matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	result = _dev->CreateDescriptorHeap(&matDescHeapDesc, IID_PPV_ARGS(materialDescHeap.ReleaseAndGetAddressOf()));
//
//	// マテリアル用ビューの作成
//	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
//	matCBVDesc.BufferLocation	= materialBuff->GetGPUVirtualAddress();	// バッファ―アドレス
//	matCBVDesc.SizeInBytes		= (UINT)materialBuffSize;						// マテリアルの256アライメントサイズ
//	
//	// 通常テクスチャビュー作成
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;				// デフォルト
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	//
//	srvDesc.ViewDimension			= D3D12_SRV_DIMENSION_TEXTURE2D;			// 2Dテクスチャ
//	srvDesc.Texture2D.MipLevels		= 1;										// ミップマップは使用しないので１
//
//	// 先頭を記録
//	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH(materialDescHeap->GetCPUDescriptorHandleForHeapStart());
//	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//	{/*
//		auto matDescHeapH = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
//	*/}
//	
//	for (unsigned int i = 0; i < materialNum; ++i) {
//		// マテリアル固定バッファビュー
//		_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
//		matDescHeapH.Offset(incSize);
//		//matDescHeapH.ptr += incSize;
//		matCBVDesc.BufferLocation += materialBuffSize;
//
//		// シェーダ―リソースビュー
//
//		// テクスチャ用ビューの作成
//		if (textureResources[i] == nullptr)
//		{
//			srvDesc.Format = whiteTex->GetDesc().Format;
//			_dev->CreateShaderResourceView(whiteTex.Get(), &srvDesc, matDescHeapH);
//		}
//		else {
//			srvDesc.Format = textureResources[i]->GetDesc().Format;
//			_dev->CreateShaderResourceView(textureResources[i].Get(), &srvDesc, matDescHeapH);
//		}
//		matDescHeapH.Offset(incSize);
//		//matDescHeapH.ptr += incSize;
//
//		// スフィアマップ用ビューの作成
//		if (sphResources[i] == nullptr) {
//			srvDesc.Format = whiteTex->GetDesc().Format;
//			_dev->CreateShaderResourceView(whiteTex.Get(), &srvDesc, matDescHeapH);
//		}
//		else {
//			srvDesc.Format = sphResources[i]->GetDesc().Format;
//			_dev->CreateShaderResourceView(sphResources[i].Get(), &srvDesc, matDescHeapH);
//		}
//		matDescHeapH.Offset(incSize);
//		//matDescHeapH.ptr += incSize;
//
//		// 加算スフィアマップ用ビューの作成
//		if (spaResources[i] == nullptr) {
//			srvDesc.Format = blackTex->GetDesc().Format;
//			_dev->CreateShaderResourceView(blackTex.Get(), &srvDesc, matDescHeapH);
//		}
//		else {
//			srvDesc.Format = spaResources[i]->GetDesc().Format;
//			_dev->CreateShaderResourceView(spaResources[i].Get(), &srvDesc, matDescHeapH);
//		}
//		matDescHeapH.Offset(incSize);
//		//matDescHeapH.ptr += incSize;
//
//		if (toonResources[i] == nullptr) {
//			srvDesc.Format = gradTex->GetDesc().Format;
//			_dev->CreateShaderResourceView(gradTex.Get(), &srvDesc, matDescHeapH);
//		}
//		else {
//			srvDesc.Format = toonResources[i]->GetDesc().Format;
//			_dev->CreateShaderResourceView(toonResources[i].Get(), &srvDesc, matDescHeapH);
//		}
//		matDescHeapH.Offset(incSize);
//		//matDescHeapH.ptr += incSize;
//
//	}
//
//	// シェーダ―
//	ID3DBlob* _vsBlob = nullptr;
//	ID3DBlob* _psBlob = nullptr;
//	ID3DBlob* errorBlob = nullptr;
//
//	// 頂点シェーダーの読み込み
//	result = D3DCompileFromFile(
//		L"BasicVertexShader.hlsl",
//		nullptr,
//		D3D_COMPILE_STANDARD_FILE_INCLUDE,
//		"BasicVS",
//		"vs_5_0",
//		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
//		0,
//		&_vsBlob,
//		&errorBlob);
//	if (FAILED(result)) {
//		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
//			::OutputDebugStringA("ファイルが見当たりません");
//		}
//		else {
//			errorDebug(errorBlob);
//		}
//		exit(1);
//	}
//
//	// ピクセルシェーダーの読み込み
//	result = D3DCompileFromFile(
//		L"BasicPixelShader.hlsl",
//		nullptr,
//		D3D_COMPILE_STANDARD_FILE_INCLUDE,
//		"BasicPS",
//		"ps_5_0",
//		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
//		0,
//		&_psBlob,
//		&errorBlob);
//	if (FAILED(result)) {
//		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
//			::OutputDebugStringA("ファイルが見当たりません");
//		}
//		else {
//			errorDebug(errorBlob);
//		}
//		exit(1);
//	}
//
//	// インプットレイアウト
//	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
//		{"POSITION",	0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//		{"NORMAL",		0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//		{"TEXCOORD",	0,DXGI_FORMAT_R32G32_FLOAT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//		{"BONE_NO",		0,DXGI_FORMAT_R16G16_UINT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//		{"WEIGHT",		0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//		//{"EDGE_FLG",	0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
//	};
//
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
//	gpipeline.pRootSignature		= nullptr;
//	gpipeline.VS = CD3DX12_SHADER_BYTECODE(_vsBlob);
//	gpipeline.PS = CD3DX12_SHADER_BYTECODE(_psBlob);
//	{/*
//		gpipeline.VS.pShaderBytecode	= _vsBlob->GetBufferPointer();
//		gpipeline.VS.BytecodeLength		= _vsBlob->GetBufferSize();
//		gpipeline.PS.pShaderBytecode	= _psBlob->GetBufferPointer();
//		gpipeline.PS.BytecodeLength		= _psBlob->GetBufferSize();
//	*/}
//
//	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
//
//	// 貼るシェーダ、ドメインシェーダー、ジオメトリシェーダーは設定しない
//	gpipeline.HS.BytecodeLength		= 0;
//	gpipeline.HS.pShaderBytecode	= nullptr;
//	gpipeline.DS.BytecodeLength		= 0;
//	gpipeline.DS.pShaderBytecode	= nullptr;
//	gpipeline.GS.BytecodeLength		= 0;
//	gpipeline.GS.pShaderBytecode	= nullptr;
//
//	// ラスタライザ(RS)
//	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
//	{/*
//		gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	// 中身を塗りつぶす
//		gpipeline.RasterizerState.FrontCounterClockwise = false;
//		gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
//		gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
//		gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
//		gpipeline.RasterizerState.DepthClipEnable = true;			// 深度方向のクリッピングを有効にする
//		gpipeline.RasterizerState.MultisampleEnable = false;		// アンチエイリアシングを使用しない
//		gpipeline.RasterizerState.AntialiasedLineEnable = false;
//		gpipeline.RasterizerState.ForcedSampleCount = 0;
//		gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
//		gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
//		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 三角形で構成
//	*/}
//
//	// OutputMarger部分
//	// レンダーターゲット
//	gpipeline.NumRenderTargets = 1;// このターゲット数と設定するファーマっと数は一致させる
//	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	// 0~1に正規化されたRGBA
//
//	// 深度ステンシル
//	gpipeline.DepthStencilState.DepthEnable		= true;
//	gpipeline.DepthStencilState.StencilEnable	= false;
//	gpipeline.DSVFormat							= DXGI_FORMAT_D32_FLOAT;
//	gpipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS;
//	gpipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
//
//	//ブレンド設定
//	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//	{/*
//		gpipeline.BlendState.AlphaToCoverageEnable		= false;
//		gpipeline.BlendState.IndependentBlendEnable		= false;
//		gpipeline.BlendState.RenderTarget->BlendEnable	= true;
//		gpipeline.BlendState.RenderTarget->SrcBlend		= D3D12_BLEND_SRC_ALPHA;
//		gpipeline.BlendState.RenderTarget->DestBlend	= D3D12_BLEND_INV_SRC_ALPHA;
//		gpipeline.BlendState.RenderTarget->BlendOp		= D3D12_BLEND_OP_ADD;
//	*/}
//	
//	gpipeline.NodeMask				= 0;
//	gpipeline.SampleDesc.Count		= 1;	// サンプリングは１ピクセルにつき１
//	gpipeline.SampleDesc.Quality	= 0;
//	gpipeline.SampleMask			= 0xffffffff;// 全部対象
//	gpipeline.Flags					= D3D12_PIPELINE_STATE_FLAG_NONE;
//
//	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
//
//	// ひとまず加算や乗算やαブレンディングは使用しない
//	renderTargetBlendDesc.BlendEnable			= false;
//	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
//	renderTargetBlendDesc.LogicOpEnable			= false;
//
//	gpipeline.BlendState.RenderTarget[0]		= renderTargetBlendDesc;
//
//	gpipeline.InputLayout.pInputElementDescs	= inputLayout;		// レイアウト先頭アドレス
//	gpipeline.InputLayout.NumElements			= _countof(inputLayout);	// レイアウト配列数
//
//	gpipeline.IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	// ストリップ時のカットなし
//	gpipeline.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で構成
//	gpipeline.NumRenderTargets					= 1;							//今は１つのみ
//	gpipeline.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;	//0～1に正規化されたRGBA
//
//	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
//	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
//	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
//
//	// レンジ
//	CD3DX12_DESCRIPTOR_RANGE descTblRange[3] = {};	// テクスチャと定数の２つ
//	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // 定数[b0]（座標変換用）
//	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // 定数[b1]（マテリアル用）
//	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); // テクスチャ4つ
//	{/*
//		// 定数用ディスクリプタレンジ
//		D3D12_DESCRIPTOR_RANGE descTblRange[3] = {};	// テクスチャと定数の２つ
//		
//		// 定数一つ目（座標変換用）
//		descTblRange[0].NumDescriptors						= 1;	//	定数ひとつ
//		descTblRange[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
//		descTblRange[0].BaseShaderRegister					= 0;	// 0番スロットから
//		descTblRange[0].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//		
//		// 定数二つ目（マテリアル用）
//		descTblRange[1].NumDescriptors						= 1;	//	ディスクリプタヒープは複数だが一度に使うのは一つ
//		descTblRange[1].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
//		descTblRange[1].BaseShaderRegister					= 1;	// 1番スロットから
//		descTblRange[1].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//		// テクスチャ一つ目（↑のマテリアルとペア）
//		descTblRange[2].NumDescriptors						= 4;	//	テクスチャ4つ(基本とsphとspaとtoon)
//		descTblRange[2].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// 種別はテクスチャ
//		descTblRange[2].BaseShaderRegister					= 0;	// 0番スロットから
//		descTblRange[2].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//	*/}
//
//
//	// ルートパラメーター
//	CD3DX12_ROOT_PARAMETER rootparam[2] = {};
//	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);	// 座標変換
//	rootparam[1].InitAsDescriptorTable(2, &descTblRange[1]);	// マテリアル周り
//	{/*
//		D3D12_ROOT_PARAMETER rootparam[2] = {};
//		rootparam[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		rootparam[0].DescriptorTable.pDescriptorRanges		= &descTblRange[0];	// ディスクリプタレンジのアドレス
//		rootparam[0].DescriptorTable.NumDescriptorRanges	= 1;	// ディスクリプタレンジ数
//		rootparam[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY_ALL;	// 全てのシェーダーから見える
//
//		rootparam[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		rootparam[1].DescriptorTable.pDescriptorRanges		= &descTblRange[1];	// ディスクリプタレンジのアドレス
//		rootparam[1].DescriptorTable.NumDescriptorRanges	= 2;	// ディスクリプタレンジ数
//		rootparam[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;	// ピクセルシェーダーから見える
//	*/}
//
//	rootSignatureDesc.pParameters = rootparam;	// ルートパラメータの先頭アドレス
//	rootSignatureDesc.NumParameters = 2;		// ルートパラメータ数
//
//	// サンプラー
//	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
//	samplerDescs[0].Init(0);
//	samplerDescs[1].Init(
//		1,
//		D3D12_FILTER_ANISOTROPIC,
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
//	{/*
//		D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
//		samplerDesc[0].AddressU				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 横繰り返し
//		samplerDesc[0].AddressV				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 縦繰り返し
//		samplerDesc[0].AddressW				= D3D12_TEXTURE_ADDRESS_MODE_WRAP;				// 奥繰り返し
//		samplerDesc[0].BorderColor			= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	// ボーダーの時は黒
//		samplerDesc[0].Filter				= D3D12_FILTER_MIN_MAG_MIP_POINT;				// 補間しない（ニアレストネイバー）
//		samplerDesc[0].MaxLOD				= D3D12_FLOAT32_MAX;							// ミップマップ最大値
//		samplerDesc[0].MinLOD				= 0.0f;											// ミップマップ最小値
//		samplerDesc[0].ComparisonFunc		= D3D12_COMPARISON_FUNC_NEVER;					// オーバーサンプリングの際リサンプリングしない
//		samplerDesc[0].ShaderVisibility		= D3D12_SHADER_VISIBILITY_PIXEL;				// ピクセルシェーダーからのみ可視
//
//		samplerDesc[1] = samplerDesc[0];	// 変更点以外をコピー
//		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
//		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
//		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	// 繰り返さない
//		samplerDesc[1].ShaderRegister = 1;							// シェーダ―スロット番号を忘れないように
//	*/}
//
//	rootSignatureDesc.pStaticSamplers	= samplerDescs;
//	rootSignatureDesc.NumStaticSamplers = 2;
//
//	{/* コード視認性は低下するので、微妙かもしれない・・・
//		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
//		rootSignatureDesc.Init(
//			2,
//			rootparam,
//			2,
//			samplerDesc,
//			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//	*/}
//
//	ID3DBlob* rootSigBlob = nullptr;
//	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
//	errorDebug(errorBlob);
//	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));
//	rootSigBlob->Release();
//
//	gpipeline.pRootSignature = rootsignature.Get();
//	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
//	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));
//
//	CD3DX12_VIEWPORT viewport(_backBuffers[0].Get());
//	{/*
//		D3D12_VIEWPORT viewport = {};
//		viewport.Width		= window_width;		// 出力先の幅（ピクセル数）
//		viewport.Height		= window_height;	// 出力先の高さ（ピクセル数）
//		viewport.TopLeftX	= 0;				// 出力先の左上座標X
//		viewport.TopLeftY	= 0;				// 出力先の左上座標Y
//		viewport.MaxDepth	= 1.0f;				// 深度最大値
//		viewport.MinDepth	= 0.0f;				// 深度最小値
//	*/}
//
//
//	CD3DX12_RECT scissorrect(0, 0, window_width, window_height);
//	{/*
//		D3D12_RECT scissorrect = {};
//		scissorrect.top		= 0;								// 切り抜き上座標
//		scissorrect.left	= 0;								// 切り抜き左座標
//		scissorrect.right	= scissorrect.left + window_width;	// 切り抜き右座標
//		scissorrect.bottom	= scissorrect.top + window_height;	// 切り抜き下座標
//	*/}
//
//	//シェーダ側に渡すための基本的な行列データ
//	struct SceneData {
//		XMMATRIX world;	// ワールド行列
//		XMMATRIX view;	// ビュー行列
//		XMMATRIX proj;	// プロジェクション行列
//		XMFLOAT3 eye;	// 視点座標
//	};
//
//	// 定数バッファ作成
//	XMMATRIX worldMat = XMMatrixIdentity();
//	XMFLOAT3 eye(0, 17, -5);
//	XMFLOAT3 target(0, 17, 0);
//	XMFLOAT3 up(0, 1, 0);
//	auto viewMat = XMMatrixLookAtLH(
//		XMLoadFloat3(&eye),
//		XMLoadFloat3(&target),
//		XMLoadFloat3(&up)
//	);
//	auto projMat = XMMatrixPerspectiveFovLH(
//		XM_PIDIV2,	//画角は90°
//		static_cast<float>(window_width) / static_cast<float>(window_height),	//アス比
//		1.0f,
//		100.0f
//	);
//
//	ComPtr<ID3D12Resource> constBuff	= nullptr;
//	auto heapPropTypeUpload		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	auto matirxCBufferDesc		= CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);
//	result = _dev->CreateCommittedResource(
//		&heapPropTypeUpload,
//		D3D12_HEAP_FLAG_NONE,
//		&matirxCBufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(constBuff.ReleaseAndGetAddressOf())
//	);
//
//	SceneData* mapMatrix = {};	// マップ先を示すポインタ
//	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);	//マップ
//	//行列の内容をコピー
//	mapMatrix->world = worldMat;
//	mapMatrix->view = viewMat;
//	mapMatrix->proj = projMat;
//	mapMatrix->eye = eye;
//
//	ComPtr<ID3D12DescriptorHeap> basicDescHeap		= nullptr;
//	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
//	descHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// シェーダ―から見える
//	descHeapDesc.NodeMask		= 0;
//	descHeapDesc.NumDescriptors = 1;	// CBV１つ
//	descHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(basicDescHeap.ReleaseAndGetAddressOf()));
//
//	// ディスクリプタの先頭アドレスを取得
//	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
//	auto basicHeapHandle	= basicDescHeap->GetCPUDescriptorHandleForHeapStart();
//	cbvDesc.BufferLocation	= constBuff->GetGPUVirtualAddress();
//	cbvDesc.SizeInBytes		= (UINT)constBuff->GetDesc().Width;
//	// 定数バッファビューの作成
//	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
//
//	MSG msg				= {};
//	unsigned int frame	= 0;
//	float angle			= 0.0f;
//	float delta			= 0.005f;
//	float sumDelta		= 0.0f;
//	while (true)
//	{
//		worldMat			= XMMatrixRotationY(angle);
//		viewMat				= XMMatrixLookAtLH(
//			XMLoadFloat3(&eye),
//			XMLoadFloat3(&target),
//			XMLoadFloat3(&up)
//		);;
//		mapMatrix->world	= worldMat;
//		mapMatrix->view		= viewMat;
//		mapMatrix->proj		= projMat;
//		mapMatrix->eye		= eye;
//
//		//angle		+= delta*10.0f;
//		//sumDelta	+= delta;
//		//eye.x		+= delta*20.0f;
//		//target.x	+= delta*20.0f;
//		//if (fabsf(sumDelta) >= 1.0f)
//		//{
//		//	delta *= -1.0f;
//		//}
//
//		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//		if (msg.message == WM_QUIT) {
//			break;
//		}
//
//		// DirectX処理
//		// バックバッファのインデックスを取得
//		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
//
//		D3D12_RESOURCE_BARRIER BarrierDesc = {};
//		BarrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//		BarrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
//		BarrierDesc.Transition.pResource	= _backBuffers[bbIdx].Get();
//		BarrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
//		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
//
//		_cmdList->SetPipelineState(_pipelinestate.Get());
//
//		// レンダーターゲットを指定
//		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
//		rtvH.ptr += static_cast<unsigned long long>(bbIdx) * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//		auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
//
//		_cmdList->ResourceBarrier(1, &BarrierDesc);
//		_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
//
//		// 画面クリア
//		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
//		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
//		_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//
//		_cmdList->RSSetViewports(1, &viewport);
//		_cmdList->RSSetScissorRects(1, &scissorrect);
//
//		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//		_cmdList->IASetVertexBuffers(0, 1, &vbView);
//		_cmdList->IASetIndexBuffer(&ibView);
//
//		_cmdList->SetGraphicsRootSignature(rootsignature.Get());
//
//		// WVP変換行列
//		_cmdList->SetDescriptorHeaps(1, basicDescHeap.GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());
//
//		// マテリアル
//		_cmdList->SetDescriptorHeaps(1, materialDescHeap.GetAddressOf());
//
//		auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();	// ヒープ先頭
//		unsigned int idxOffset = 0;
//
//		auto cbvsrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)* 5;
//		for (auto& m : materials) {
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialH);
//			_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
//			// ヒープポインタとインデックスを次に進める
//			materialH.ptr += cbvsrvIncSize;
//			idxOffset += m.indicesNum;
//		}
//
//		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
//		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
//
//		_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//		//命令のクローズ
//		_cmdList->Close();
//
//		// コマンドリストの実行
//		ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
//		_cmdQueue->ExecuteCommandLists(1, cmdlists);
//		// 待ち
//		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
//
//		while (_fence->GetCompletedValue() != _fenceVal) {
//			;
//		}
//
//		// フリップ
//		_swapchain->Present(1, 0);
//		_cmdAllocator->Reset();							//キューをクリア
//		_cmdList->Reset(_cmdAllocator.Get(), nullptr);	//再びコマンドリストをためる準備
//
//	}
//	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
//
//#ifdef _DEBUG
//	ReportD3DObject();
//#endif
//
//	// 解放
//	//safeRelease(gradTex);
//	//safeRelease(blackTex);
//	//safeRelease(whiteTex);
//	//for (auto toon : toonResources)
//	//{
//	//	safeRelease(toon);
//	//}
//	//for (auto spa : spaResources)
//	//{
//	//	safeRelease(spa);
//	//}
//	//for (auto sph : sphResources)
//	//{
//	//	safeRelease(sph);
//	//}
//	//for (auto tex : textureResources)
//	//{
//	//	safeRelease(tex);
//	//}
//	//safeRelease(materialDescHeap);
//	//safeRelease(materialBuff);
//	//safeRelease(basicDescHeap);
//	//safeRelease(constBuff);
//	//safeRelease(_pipelinestate);
//	//safeRelease(rootsignature);
//	//safeRelease(idxBuff);
//	//safeRelease(vertBuff);
//	//safeRelease(_fence);
//	//safeRelease(dsvHeap);
//	//safeRelease(depthBuffer);
//	//for (auto buf : _backBuffers)
//	//{
//	//	safeRelease(buf);
//	//}
//	//safeRelease(rtvHeaps);
//	//releaseResource();
//
//#ifdef _DEBUG
//	ReportD3DObject();
//#endif
//	//safeRelease(_dev);
//	return 0;
//}
