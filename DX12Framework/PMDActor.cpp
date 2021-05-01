#include "PMDActor.h"
#include "PMDRenderer.h"
#include "Dx12Wrapper.h"
#include <d3dx12.h>
using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

namespace {
	// テクスチャのパスをセパレータ文字で分離する
	// @param path		対象のパス文字列
	// @param splitter	区切り文字
	// @return			分離前後の文字列ペア
	std::pair<string, string> 
		SplitFileName(const std::string& path, const char splitter = '*')
	{
		size_t idx = path.find(splitter);
		pair<string, string> ret;
		ret.first = path.substr(0, idx);
		ret.second = path.substr(idx + 1, path.length() - idx - 1);
		return ret;
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
}

void* PMDActor::Transform::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

PMDActor::PMDActor(const char* filepath, PMDRenderer& renderer):
	_renderer(renderer),
	_dx12(renderer._dx12),
	_angle(0.0f)
{
	_transform.world = XMMatrixIdentity();
	LoadPMDFile(filepath);
	CreateTransformView();
	CreateMaterialData();
	CreateMaterialAndTextureView();
}

PMDActor::~PMDActor()
{
}

// PMDファイルのロード
HRESULT PMDActor::LoadPMDFile(const char* path)
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

	// 頂点数の読み込み
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

	// 頂点データの読み込み
	constexpr unsigned int pmdvertex_size	= 38;		// 頂点１つあたりのサイズ
	unsigned int verticesSize				= vertNum * pmdvertex_size;
	std::vector<unsigned char> vertices(verticesSize);	// バッファ確保
	fread(vertices.data(), vertices.size(), 1, fp);		// 読み込み

	// インデックス数の取得
	unsigned int indicesNum;// インデックス数
	fread(&indicesNum, sizeof(indicesNum), 1, fp);

	// 頂点バッファの作成
	// UPLOAD(確保は可能)
	auto vertHeapProp	= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());
	auto result = _dx12.Device()->CreateCommittedResource(
		&vertHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_vb.ReleaseAndGetAddressOf()));

	// 作ったバッファに頂点データをコピー
	unsigned char* vertMap = nullptr;
	result = _vb->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vb->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	_vbView.BufferLocation	= _vb->GetGPUVirtualAddress();	// バッファの仮想アドレス
	_vbView.SizeInBytes		= (UINT)vertices.size();		// 全バイト数
	_vbView.StrideInBytes	= pmdvertex_size;				// 1頂点あたりのバイト数

	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);	// 一気に読み込み

	// インデックスバッファーの作成
	// 設定は、バッファのサイズ以外頂点バッファの設定を使いまわしてOK
	auto indexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));
	result = _dx12.Device()->CreateCommittedResource(
		&indexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_ib.ReleaseAndGetAddressOf()));

	// 作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	_ib->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_ib->Unmap(0, nullptr);

	// インデックスバッファビューを作成
	_ibView.BufferLocation	= _ib->GetGPUVirtualAddress();
	_ibView.Format			= DXGI_FORMAT_R16_UINT;
	_ibView.SizeInBytes		= (UINT)indices.size() * sizeof(indices[0]);

	// マテリアル数を読み込み
	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);

	// リソースサイズを設定
	_materials.resize(materialNum);
	_textureResources.resize(materialNum);
	_sphResources.resize(materialNum);
	_spaResources.resize(materialNum);
	_toonResources.resize(materialNum);

	// PMDマテリアル情報の読み込み
	std::vector<PMDMaterial> pmdMaterials(materialNum);
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

	for (int i = 0; i < pmdMaterials.size(); ++i) {
		// トゥーンリソースの読み込み
		char toonFilePath[32];
		sprintf(toonFilePath, "toon/toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
		_toonResources[i] = _dx12.GetTextureByPath(toonFilePath);

		if (strlen(pmdMaterials[i].texFilePath) == 0) {
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
			_textureResources[i] = _dx12.GetTextureByPath(texFilePath.c_str());
		}
		// 乗算スフィアマップの読み込み
		if (sphFileName != "") {
			auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
			_sphResources[i] = _dx12.GetTextureByPath(sphFilePath.c_str());
		}
		// 加算スフィアマップの読み込み
		if (spaFileName != "") {
			auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
			_spaResources[i] = _dx12.GetTextureByPath(spaFilePath.c_str());
		}
	}
	fclose(fp);

	return result;
}

// 座標変換用ビューの生成
HRESULT PMDActor::CreateTransformView() {
	// GPUバッファ作成
	auto buffSize = sizeof(Transform);
	buffSize = (buffSize + 0xff)&~0xff;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);
	auto result = _dx12.Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	// マップとコピー
	result = _transformBuff->Map(0, nullptr, (void**)&_mappedTransform);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}
	*_mappedTransform = _transform;

	// ビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;	// とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;// デスクリプタヒープ種別
	result = _dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf()));// 生成
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)buffSize;
	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

// マテリアルバッファを作成
HRESULT PMDActor::CreateMaterialData() {
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto materialResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * _materials.size());// もったいない
	auto result = _dx12.Device()->CreateCommittedResource(
		&materialHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&materialResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_materialBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	// マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = _materialBuffer->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}
	for (auto& m : _materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material; // データコピー
		mapMaterial += materialBuffSize; // 次のアラインメント位置まで進める
	}
	_materialBuffer->Unmap(0, nullptr);

	return S_OK;

}

// マテリアル＆テクスチャのビューを作成
HRESULT PMDActor::CreateMaterialAndTextureView() {
	// マテリアル用ディスクリプタヒープとビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
	matDescHeapDesc.NumDescriptors = (UINT)(_materials.size() * 5);	// マテリアル数(定数1つ,テクスチャ3つ)を指定
	matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDescHeapDesc.NodeMask = 0;
	matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;// デスクリプタヒープ種別

	auto result = _dx12.Device()->CreateDescriptorHeap(
		&matDescHeapDesc,
		IID_PPV_ARGS(_materialDescHeap.ReleaseAndGetAddressOf())); // 生成

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	// マテリアルバッファサイズを取得
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	// マテリアル用ビューの作成
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation	= _materialBuffer->GetGPUVirtualAddress();	// バッファ―アドレス
	matCBVDesc.SizeInBytes		= (UINT)materialBuffSize;					// マテリアルの256アライメントサイズ

	// 通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;				// デフォルト
	srvDesc.Shader4ComponentMapping		= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension				= D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels			= 1;//ミップレベルは使用しないので１

	// 先頭を記録
	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH(_materialDescHeap->GetCPUDescriptorHandleForHeapStart());
	auto incSize = _dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (unsigned int i = 0; i < _materials.size(); ++i) {
		// マテリアル固定バッファビュー
		_dx12.Device()->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.Offset(incSize);
		matCBVDesc.BufferLocation += materialBuffSize;

		// テクスチャ用ビューの作成
		if (_textureResources[i] == nullptr)
		{
			srvDesc.Format = _renderer._whiteTex->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_renderer._whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _textureResources[i]->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_textureResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);

		// スフィアマップ用ビューの作成
		if (_sphResources[i] == nullptr) {
			srvDesc.Format = _renderer._whiteTex->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_renderer._whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _sphResources[i]->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_sphResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);

		// 加算スフィアマップ用ビューの作成
		if (_spaResources[i] == nullptr) {
			srvDesc.Format = _renderer._blackTex->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_renderer._blackTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _spaResources[i]->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_spaResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);

		if (_toonResources[i] == nullptr) {
			srvDesc.Format = _renderer._gradTex->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_renderer._gradTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _toonResources[i]->GetDesc().Format;
			_dx12.Device()->CreateShaderResourceView(_toonResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);
	}

	return result;
}

// 更新
void PMDActor::Update() {
	_angle += 0.03f;
	_mappedTransform->world = XMMatrixRotationY(_angle);
}

// 描画
void PMDActor::Draw() {
	_dx12.CommandList()->IASetVertexBuffers(0, 1, &_vbView);
	_dx12.CommandList()->IASetIndexBuffer(&_ibView);

	ID3D12DescriptorHeap* transheaps[] = { _transformHeap.Get() };
	_dx12.CommandList()->SetDescriptorHeaps(1, transheaps);
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(0, _transformHeap->GetGPUDescriptorHandleForHeapStart());


	ID3D12DescriptorHeap* mdh[] = { _materialDescHeap.Get() };
	// マテリアル
	_dx12.CommandList()->SetDescriptorHeaps(1, mdh);

	auto materialH = _materialDescHeap->GetGPUDescriptorHandleForHeapStart();	// ヒープ先頭
	unsigned int idxOffset = 0;


	auto cbvsrvIncSize = _dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
	for (auto& m : _materials) {
		_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, materialH);
		_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		// ヒープポインタとインデックスを次に進める
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}

}