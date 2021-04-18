#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<map>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>
#include<dxgidebug.h>
#include<wrl.h>

class Application
{
private:
	// ここに必要な変数(バッファーやヒープなど)を書く
	// ウィンドウ周り
	WNDCLASSEX windowClass = {};
	HWND hwnd;
	// DXGIまわり
	Microsoft::WRL::ComPtr<IDXGIFactory4>				_dxgiFactory	= nullptr;	// DXGIインターフェイス
	Microsoft::WRL::ComPtr<IDXGISwapChain4>				_swapchain		= nullptr;	// スワップチェイン

	// DirectX12まわり
	Microsoft::WRL::ComPtr<ID3D12Device>				_dev			= nullptr;	// デバイス
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		_cmdAllocator	= nullptr;	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	_cmdList		= nullptr;	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			_cmdQueue		= nullptr;	// コマンドキュー

	// 必要最低限のバッファまわり
	Microsoft::WRL::ComPtr<ID3D12Resource>				_depthBuffer	= nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>				_vertBuffer		= nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>				_idxBuffer		= nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>				_constBuffer	= nullptr;

	// ロード用テーブル
	using LoadLambda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map < std::string, LoadLambda_t> _loadLambdaTable;

	// マテリアル周り
	unsigned int materialNum;
	Microsoft::WRL::ComPtr<ID3D12Resource>				_materialBuffer = nullptr;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};

	// デフォルトのテクスチャ（白、黒、グレイスケールグラデーション）
	Microsoft::WRL::ComPtr<ID3D12Resource> _whiteTex = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _balckTex = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _gradTex = nullptr;

	// 座標変換系行列
	DirectX::XMMATRIX _worldMat;
	DirectX::XMMATRIX _viewMat;
	DirectX::XMMATRIX _projMat;

	// シェーダ―側に投げられるマテリアルデータ
	struct MaterialForHlsl {
		DirectX::XMFLOAT3 diffuse;	// ディフューズ色
		float alpha;		// ディフューズα
		DirectX::XMFLOAT3 specular;	// スペキュラ色
		float specularity;	// スペキュラの強さ（乗算値）
		DirectX::XMFLOAT3 ambient;	// アンビエント色
		MaterialForHlsl()
		{
			alpha = 0.0f;
			specular = {};
			specularity = 0.0f;
			ambient = {};
		}
	};
	// それ以外のマテリアルデータ
	struct AdditionalMaterial {
		std::string texPath;	// テクスチャファイルパス
		int toonIdx;			// トゥーン番号
		bool edgeFlg;			// マテリアル毎の輪郭線フラグ
		AdditionalMaterial() {
			texPath = {};
			toonIdx = 0;
			edgeFlg = 0;
		}
	};
	// まとめたもの
	struct Material {
		unsigned int indicesNum;	// インデックス数
		MaterialForHlsl material;
		AdditionalMaterial additional;
		Material(){
			indicesNum = 0;
			material = {};
			additional = {};
		}
	};
	std::vector<Material> _materials;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _textureResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _sphResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _spaResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _toonResources;

	//シェーダ側に渡すための基本的な行列データ
	struct SceneData {
		DirectX::XMMATRIX world;	// ワールド行列
		DirectX::XMMATRIX view;		// ビュー行列
		DirectX::XMMATRIX proj;		// プロジェクション行列
		DirectX::XMFLOAT3 eye;		// 視点座標
	};
	SceneData* _mapScene;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _basicDescHeap		= nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _materialDescHeap	= nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;

	// 頂点＆インデックスバッファビュー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	// ファイル名パスとリソースのマップテーブル
	std::map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>> _resourceTable;

	// パイプライン＆ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootsignature = nullptr;

	std::vector<ID3D12Resource*> _backBuffers;	// バックバッファ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;	// レンダーターゲット用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeaps = nullptr;	// 深度バッファビュー用デスクリプタヒープ
	CD3DX12_VIEWPORT	_viewport;		// ビューポート
	CD3DX12_RECT		_scissorrect;	// シザー矩形

#ifdef _DEBUG
	// デバッグ用エラー出力ラムダ
	std::function<void(ID3DBlob* errorBlob)> errorDebug = [](ID3DBlob* errorBlob)
	{
		if (errorBlob == nullptr)return;
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		OutputDebugStringA(errstr.c_str());
	};
#endif

	// テクスチャバッファ周り
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateWhiteTexture();						// 白テクスチャの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBlackTexture();						// 黒テクスチャの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateGrayGradationTexture();				// グレーテクスチャの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);	// 指定テクスチャのロード

	// 最終的なレンダーターゲットの生成
	HRESULT CreateFinalRenderTarget(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& rtvHeaps, std::vector<ID3D12Resource*>& backBuffers);

	// スワップチェインの生成
	HRESULT CreateSwapChain(const HWND& hwnd, Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory);

	// ゲーム用ウィンドウの生成
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	// DXGIまわり初期化
	HRESULT InitializeDXGIDevice();

	// コマンドまわり初期化
	HRESULT InitializeCommand();

	// パイプライン初期化
	HRESULT CreateBassicGraphicsPipeline();
	// ルートシグネチャ初期化
	HRESULT CreateRootSignature();

	// テクスチャローダテーブルの作成
	void CreateTextureLoaderTable();

	// デプスステンシルビューの生成
	HRESULT CreateDepthStencilView();

	// PMDファイルのロード
	HRESULT LoadPMDFile(const char* path);

	// GPU側のマテリアルデータの作成
	HRESULT CreateMaterialData();

	// 座標変換用ビューの生成
	HRESULT CreateSceneTransformView();

	// マテリアル＆テクスチャのビューを作成
	void CreateMaterialAndTextureView();

	// シングルトンのためにコンストラクタをprivateに
	// さらにコピーと代入を禁止する
	Application();
	Application(const Application&) = delete;
	void operator = (const Application&) = delete;

public:
	// Applicaitonのシングルインスタンスを得る
	static Application& Instance();

	// 初期化
	bool Init();

	// ループ起動
	void Run();

	// 後処理
	void Terminate();

	~Application();
};
