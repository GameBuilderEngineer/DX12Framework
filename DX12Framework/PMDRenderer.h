#pragma once

#include<d3d12.h>
#include<vector>
#include<wrl.h>
#include<memory>

class Dx12Wrapper;
class PMDActor;
class PMDRenderer
{
	friend PMDActor;
private:
	Dx12Wrapper& _dx12;
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr <T>;

	// パイプライン＆ルートシグネチャ
	ComPtr<ID3D12PipelineState> _pipeline		= nullptr;	// PMD用パイプライン
	ComPtr<ID3D12RootSignature> _rootsignature	= nullptr;	// PMD用ルートシグネチャ

	// PMD用共通テクスチャ（白、黒、グレイスケールグラデーション）
	ComPtr<ID3D12Resource> _whiteTex	= nullptr;
	ComPtr<ID3D12Resource> _blackTex	= nullptr;
	ComPtr<ID3D12Resource> _gradTex		= nullptr;

	// テクスチャバッファ周り
	ID3D12Resource* CreateDefaultTexture(size_t width,size_t height);	// テクスチャリソースの作成
	ID3D12Resource* CreateWhiteTexture();								// 白テクスチャの生成
	ID3D12Resource* CreateBlackTexture();								// 黒テクスチャの生成
	ID3D12Resource* CreateGrayGradationTexture();						// グレーテクスチャの生成

	// パイプライン初期化
	HRESULT CreateGraphicsPipelineForPMD();
	// ルートシグネチャ初期化
	HRESULT CreateRootSignature();

	// シェーダ―コンパイル結果の確認
	bool CheckShaderCompileResult(HRESULT result, ID3DBlob* error = nullptr);

public:
	PMDRenderer(Dx12Wrapper& dx12);
	~PMDRenderer();
	void Update();
	void Draw();
	ID3D12PipelineState* GetPiplineState();
	ID3D12RootSignature* GetRootSignature();

};
