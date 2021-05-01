﻿#pragma once

#include<d3d12.h>
#include<DirectXMath.h>
#include<vector>
#include<wrl.h>

class Dx12Wrapper;
class PMDRenderer;
class PMDActor
{
	friend PMDRenderer;
private:
	PMDRenderer& _renderer;
	Dx12Wrapper& _dx12;
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 頂点関連
	ComPtr<ID3D12Resource> _vb = nullptr;
	ComPtr<ID3D12Resource> _ib = nullptr;
	// 頂点＆インデックスバッファビュー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	// シェーダ―側に投げられるマテリアルデータ
	struct MaterialForHlsl {
		DirectX::XMFLOAT3 diffuse;	// ディフューズ色
		float alpha;		// ディフューズα
		DirectX::XMFLOAT3 specular;	// スペキュラ色
		float specularity;	// スペキュラの強さ（乗算値）
		DirectX::XMFLOAT3 ambient;	// アンビエント色
		MaterialForHlsl()
		{
			diffuse		= {};
			alpha		= 0.0f;
			specular	= {};
			specularity	= 0.0f;
			ambient		= {};
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

	struct Transform
	{
		// 内部に持っているXMMATRIXメンバが16バイトアライメントであるため
		// Transformをnewする際には16バイト境界に確保する
		void* operator new(size_t size);
		DirectX::XMMATRIX world;
	};

	Transform _transform;
	Transform* _mappedTransform = nullptr;
	ComPtr<ID3D12Resource> _transformBuff = nullptr;
	
	// マテリアル周り
	std::vector<Material> _materials;
	ComPtr<ID3D12Resource> _materialBuffer = nullptr;
	std::vector<ComPtr<ID3D12Resource>> _textureResources;
	std::vector<ComPtr<ID3D12Resource>> _sphResources;
	std::vector<ComPtr<ID3D12Resource>> _spaResources;
	std::vector<ComPtr<ID3D12Resource>> _toonResources;

	// 読み込んだマテリアルをもとにマテリアルバッファを作成
	HRESULT CreateMaterialData();

	// マテリアルヒープ(5個分)
	ComPtr<ID3D12DescriptorHeap> _materialDescHeap	= nullptr;
	// マテリアル＆テクスチャのビューを作成
	HRESULT CreateMaterialAndTextureView();

	// 座標変換用ビューの生成
	HRESULT CreateTransformView();

	// PMDファイルのロード
	HRESULT LoadPMDFile(const char* path);

	float _angle;	// テスト用Y軸回転
public:
	PMDActor(const char* filepath, PMDRenderer& renderer);
	~PMDActor();

	// クローンは頂点及びマテリアルは共通のバッファを見るようにする
	PMDActor* Clone();
	void Update();
	void Draw();

};

