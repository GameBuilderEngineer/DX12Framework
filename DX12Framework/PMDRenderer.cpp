#include "PMDRenderer.h"
#include<d3dx12.h>
#include<cassert>
#include<d3dcompiler.h>
#include"Dx12Wrapper.h"
#include<string>
#include<algorithm>

using namespace std;
using Microsoft::WRL::ComPtr;

namespace {
	void PrintErrorBlob(ID3DBlob* blob) {
		if (blob == nullptr)return;
		string err;
		err.resize(blob->GetBufferSize());
		copy_n((char*)blob->GetBufferPointer(), err.size(), err.begin());
		err += "\n";
		OutputDebugStringA(err.c_str());
	}
}

PMDRenderer::PMDRenderer(Dx12Wrapper& dx12) :_dx12(dx12)
{
	assert(SUCCEEDED(CreateRootSignature()));
	assert(SUCCEEDED(CreateGraphicsPipelineForPMD()));
	_whiteTex = CreateWhiteTexture();
	DxDebug _whiteTex->SetName(L"_whiteTex");
	_blackTex = CreateBlackTexture();
	DxDebug _blackTex->SetName(L"_blackTex");
	_gradTex = CreateGrayGradationTexture();
	DxDebug _gradTex->SetName(L"_gradTex");
}

PMDRenderer::~PMDRenderer() {

}

void PMDRenderer::Update() {

}

void PMDRenderer::Draw() {

}

// テクスチャリソースの作成
ComPtr<ID3D12Resource> PMDRenderer::CreateDefaultTexture(size_t width, size_t height) {
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, (UINT)height);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	ComPtr<ID3D12Resource> buff = nullptr;
	auto result = _dx12.Device()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(buff.GetAddressOf())
	);
	DxDebug buff->SetName(L"DefaultTexture::buff");
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return nullptr;
	}
	DxDebug _dx12.ReportD3DObject();
	return buff;
}

// 白テクスチャの作成
ComPtr<ID3D12Resource> PMDRenderer::CreateWhiteTexture()
{
	ComPtr<ID3D12Resource> whiteBuff = CreateDefaultTexture(4, 4);

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);	// 全部255で埋める

	// データ転送
	auto result = whiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		(UINT)data.size()
	);
	assert(SUCCEEDED(result));
	return whiteBuff;
}

// 黒テクスチャの作成
ComPtr<ID3D12Resource> PMDRenderer::CreateBlackTexture()
{
	ComPtr<ID3D12Resource> blackBuff = CreateDefaultTexture(4,4);

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);	// 全部0で埋める

	// データ転送
	auto result = blackBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		(UINT)data.size()
	);
	assert(SUCCEEDED(result));
	return blackBuff;
}

// デフォルトグラデーションテクスチャ
ComPtr<ID3D12Resource> PMDRenderer::CreateGrayGradationTexture()
{
	ComPtr<ID3D12Resource> gradBuff = CreateDefaultTexture(4, 256);

	// 上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (c << 0xff) | (c << 16) | (c << 8) | c;
		std::fill(it, it + 4, col);
		--c;
	}

	auto result = gradBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * (UINT)data.size()
	);
	assert(SUCCEEDED(result));
	return gradBuff;
}

// シェーダ―コンパイル結果の確認
bool PMDRenderer::CheckShaderCompileResult(HRESULT result, ID3DBlob* error) {
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			PrintErrorBlob(error);
		}
		return false;
	}
	else {
		return true;
	}
}

// パイプライン初期化
HRESULT PMDRenderer::CreateGraphicsPipelineForPMD()
{
	// シェーダ―
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	// 頂点シェーダーの読み込み
	auto result = D3DCompileFromFile(
		L"BasicShader.hlsl",
		nullptr,
		nullptr,//D3D_COMPILE_STANDARD_FILE_INCLUDE：相対パスによるインクルードを可能にする
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob);
	if (!CheckShaderCompileResult(result, errorBlob.Get())) {
		assert(0);
		return result;
	}

	// ピクセルシェーダーの読み込み
	result = D3DCompileFromFile(
		L"BasicShader.hlsl",
		nullptr,
		nullptr,//D3D_COMPILE_STANDARD_FILE_INCLUDE：相対パスによるインクルードを可能にする
		"BasicPS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob);
	if (!CheckShaderCompileResult(result, errorBlob.Get())) {
		assert(0);
		return result;
	}

	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",	0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",		0,DXGI_FORMAT_R32G32B32_FLOAT,	0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",	0,DXGI_FORMAT_R32G32_FLOAT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		//{"BONE_NO",		0,DXGI_FORMAT_R16G16_UINT,		0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		//{"WEIGHT",		0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		//{"EDGE_FLG",	0,DXGI_FORMAT_R8_UINT,			0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = _rootsignature.Get();
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

	// ブレンド設定
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	// ラスタライザ(RS)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない

	// 深度ステンシル
	gpipeline.DepthStencilState.DepthEnable		= true;
	gpipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat							= DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable	= false;

	gpipeline.InputLayout.pInputElementDescs	= inputLayout;				// レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements			= _countof(inputLayout);	// レイアウト配列数

	gpipeline.IBStripCutValue		= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	// ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		//三角形で構成

	gpipeline.NumRenderTargets		= 1;							//今は１つのみ
	gpipeline.RTVFormats[0]			= DXGI_FORMAT_R8G8B8A8_UNORM;	//0～1に正規化されたRGBA

	gpipeline.SampleDesc.Count		= 1;	// サンプリングは１ピクセルにつき１
	gpipeline.SampleDesc.Quality	= 0;	// クオリティは最低

	// 貼るシェーダ、ドメインシェーダー、ジオメトリシェーダーは設定しない
	gpipeline.HS.BytecodeLength = 0;
	gpipeline.HS.pShaderBytecode = nullptr;
	gpipeline.DS.BytecodeLength = 0;
	gpipeline.DS.pShaderBytecode = nullptr;
	gpipeline.GS.BytecodeLength = 0;
	gpipeline.GS.pShaderBytecode = nullptr;

	// OutputMarger部分
	// レンダーターゲット
	gpipeline.NumRenderTargets	= 1;// このターゲット数と設定するファーマっと数は一致させる
	gpipeline.RTVFormats[0]		= DXGI_FORMAT_R8G8B8A8_UNORM;	// 0~1に正規化されたRGBA

	gpipeline.NodeMask		= 0;
	gpipeline.SampleMask	= 0xffffffff;// 全部対象
	gpipeline.Flags			= D3D12_PIPELINE_STATE_FLAG_NONE;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	// ひとまず加算や乗算やαブレンディングは使用しない
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable = false;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	result = _dx12.Device()->CreateGraphicsPipelineState(
		&gpipeline, 
		IID_PPV_ARGS(_pipeline.ReleaseAndGetAddressOf()));

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
	}

	return result;
}

// ルートシグネチャ初期化
HRESULT PMDRenderer::CreateRootSignature()
{
	// レンジ
	const int DESC_RANGE_NUM = 4;
	CD3DX12_DESCRIPTOR_RANGE descTblRange[DESC_RANGE_NUM] = {};		// テクスチャと定数の２つ
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// 定数[b0]（ビュープロジェクション用）
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	// 定数[b1]（ワールド、ボーン用）
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// 定数[b2]（マテリアル用）
	descTblRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);	// テクスチャ4つ

	// ルートパラメーター
	const int ROOT_PARAMETER_NUM = 3;
	CD3DX12_ROOT_PARAMETER rootparam[ROOT_PARAMETER_NUM] = {};
	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);	// 座標変換
	rootparam[1].InitAsDescriptorTable(1, &descTblRange[1]);	// マテリアル周り
	rootparam[2].InitAsDescriptorTable(2, &descTblRange[2]);	// マテリアル周り

	// サンプラー
	const int STATIC_SAMPLER_NUM = 2;
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[STATIC_SAMPLER_NUM] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(
		1,//シェーダ―レジスターNO
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags				= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters		= rootparam;			// ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters		= ROOT_PARAMETER_NUM;	// ルートパラメータ数
	rootSignatureDesc.pStaticSamplers	= samplerDescs;
	rootSignatureDesc.NumStaticSamplers = STATIC_SAMPLER_NUM;

	ComPtr<ID3DBlob> rootSigBlob	= nullptr;
	ComPtr<ID3DBlob> errorBlob		= nullptr;
	// ルートシグネチャのシリアライズ
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1_0, 
		&rootSigBlob, 
		&errorBlob
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}
	PrintErrorBlob(errorBlob.Get());

	// ルートシグネチャの作成
	result = _dx12.Device()->CreateRootSignature(
		0, 
		rootSigBlob->GetBufferPointer(), 
		rootSigBlob->GetBufferSize(), 
		IID_PPV_ARGS(_rootsignature.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	return result;
}

ID3D12PipelineState* PMDRenderer::GetPiplineState() {
	return _pipeline.Get();
}

ID3D12RootSignature* PMDRenderer::GetRootSignature() {
	return _rootsignature.Get();
}


