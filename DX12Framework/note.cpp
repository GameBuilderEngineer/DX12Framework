#define NOTE_MODE 0
#define CHAPTER 6


#if NOTE_MODE
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


namespace note {

	class Note
	{
	public:
		void main();

	private:
	};

#pragma region CHAPTER5
#if CHAPTER == 5

	void Note::main()
	{
		// GPU側のメモリ領域を確保するためのオブジェクト
		ID3D12Resource object;
		// ID3D12Device::CreateCommittedResource()メソッドを使用して生成する
		// * 別の方法
		//		ID3D12Device::CreateCommittedResource()メソッドではなく、ID3D12Device::CreateHeap()
		//		メソッドとID3D12Device::CreatePlacedResource()メソッドを使用して生成する方法もある。
	}

	HRESULT CreateCommittedResource(
		const D3D12_HEAP_PROPERTIES* pHeapProperties,	// ヒープ設定構造体のアドレス
		D3D12_HEAP_FLAGS HeapFlags,						// 特に指定がないためD3D12_HEAP_FLAG_NONEでよい
		const D3D12_RESOURCE_DESC* pDesc,				// リソース設定構造体のアドレス
		D3D12_RESOURCE_STATES InitialResourceState,		// GPU側からは読み取り専用なのでGENERIC_READ
		const D3D12_CLEAR_VALUE* pOptimizedClearValue,	// 使わないのでnullptrでよい
		REFIID rridResource,
		void** ppvResource
	) {
		// D3D12_HEAP_PROPERTIES構造体とD3D12_RESOURCE_DESC構造体の２つは、
		// 「ヒープの設定」と「リソースの設定」といいつつどちらもメモリを示している
	}

	// TITLE: D3D12_HEAP_PROPERTIES
	typedef struct D3D12_HEAP_PROPERTIES
	{
		D3D12_HEAP_TYPE type;						// ヒープ種別（mapする必要があるならUPLOAD）
		D3D12_CPU_PAGE_PROPERTY CPUPageProperty;	// CPUのページング設定(PAGE_PROPERTY_UNKNOWNでよい)
		D3D12_MEMORY_POOL MemoryPoolPreference;		// メモリプールがどこかを示す(POOL_UNKNOWNでよい)
		UINT CreationNodeMask;						// 単一アダプターなら0でよい
		UINT VisibleNodeMask;						// 単一アダプターなら0でよい
	}D3D12_HEAP_PROPERTIES;
	// NOTE: D3D12_HEAP_TYPE：ヒープの種別を表す
	enum D3D12_HEAP_TYPE
	{
		D3D12_HEAP_TYPE_DEFAULT,	// CPUからアクセスできない（マップできない）
		D3D12_HEAP_TYPE_UPLOAD,		// CPUからアクセスできる（マップできる）
		D3D12_HEAP_TYPE_READBACK,	// CPUから読み取れる
		D3D12_HEAP_TYPE_CUSTOM,		// カスタムヒープ
	};
	// ポリゴンを表示するために使用されるヒープは、DEFAULTやUPLOADがほとんど
	// DEFAULTは、「CPUからアクセスできない」ヒープ
	// そのため、ID3D12Resource::Map()メソッドでヒープにアクセスしようとすると失敗する
	// このヒープはバンド幅が最大に広く（つまりアクセスが速いため）、GPU側のみで扱うためのヒープとしては有用
	// UPLOADは、CPUのデータをGPUにアップロードするために使用する
	// このヒープにはCPUからアクセスでき、ID3D12Resource::Map()メソッドで中身を書き換えることが可能
	// 便利ではあるものの、CPU/GPU双方からのアクセスがD3D12_HEAP_TYPE_DEFAULTのヒープに比べると遅いため、
	// 「CPUからの１回限りの書き込み」「GPUからの１回限りの読み込み」をするような場合に推奨される
	// READBACKは、読み戻しのヒープ
	// CPU側からアクセス可能なヒープだが、バンド幅は広くない（比較的遅い）
	// GPUで加工・計算したデータをCPU側で活用するためのものといえる
	// CUSTOMは、CUSTOM以外の設定の場合、ページング設定やメモリプール設定にUNKNOWNを指定すればよいが、
	// CUSTOMではこのページング設定やメモリプールを正しく設定しなければならない
	// 頂点バッファー作成時には、DEFAULTかUPLOADを使う

	// NOTE: CPUのページング設定
	enum D3D12_CPU_PAGE_PROPERTY
	{
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,		// 考えなくてよい
		D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,	// CPUからアクセス不可
		D3D12_CPU_PAGE_PROPERTY_WROTE_COMBINE,	// ライトコンバイン
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,		// ライトバック
	};
	// UNKNOWNは、D3D12_HEAP_TYPE_CUSTOM以外のときに使う設定
	// D3D12_HEAP_TYPE_CUSTOMのときに、UNKNOWNを設定することはできない
	// NOT_AVAILABLEは、「CPUからのアクセスが不可」であり、
	// 「GPU内での計算に限定される」ということになる
	// WRITE_BACKは、「CPUのメモリをキャッシュとして使用する」もしくは、
	// 「CPU側のキャッシュメモリをキャッシュとして、時間の空きができ次第、
	// 順次GPU側のメモリに転送する」方式
	// WRITE_COMBINEは、送るべきデータをある程度の大きさのデータにまとめて転送する方式
	// この方式をバーストモードと呼んだりする
	// ただし、順序が考慮されないため、注意が必要

	// NOTE: メモリプール設定
	enum D3D12_MEMORY_POOL
	{
		D3D12_MEMORY_POOL_UNKNOWN,	// CUSTOM以外のときはこれでよい
		D3D12_MEMORY_POOL_L0,		// システムメモリ（アダプターがUMAのとき、GPUバンド幅は狭い）
		D3D12_MEMORY_POOL_L1,		// ビデオメモリ（アダプターがNUMAのとき、GPUバンド幅は広い）
	};
	// UNKOWNは、D3D12_HEAP_CUSTOM以外のときに使用する
	// L0はシステムメモリ(CPU側のメモリ)を表し、アダプター（グラフィックボード）によって
	// 挙動が変わる
	// ディスクリートグラフィックスボードを参照している場合にL0を選ぶと
	// CPUバンド幅が広く、GPUバンド幅が狭くなる
	// L1は、ビデオメモリを表す
	// オンボードGPU(UMA)の場合には使用できない
	// GPU用のバンド幅が広く、CPU側からのアクセスができなくなる

	// TITLE: リソース設定構造体
	// ヒープ設定後は、リソースの設定を行う
	typedef struct D3D12_RESOURCE_DESC
	{
		D3D12_RESOURCE_DIMENSION Dimension;	// バッファーに使うのでBUFFERを指定
		UINT64 Alignment;					// 0でよい
		UINT64 Width;						// 幅で全部まかなうのでsizof(全頂点)とする
		UINT Height;						// 幅で表現しているので１とする
		UINT16 DepthOrArraySize;			// １でよい
		UINT16 MipLevels;					// １でよい
		DXGI_FORMAT Format;					// 画像ではないのでUNKNOWNでよい
		DXGI_SMAPLE_DESC SampleDesc;		// SampleDesc.Count = 1とする
		D3D12_TEXTURE_LAYOUT Layout;		// D3D12_TEXTURE_LAYOUT_ROW_MAJORとする
		D3D12_RESOURCE_FLAGS Flags;			// NONEでよい
	} D3D12_RESOURCE_DESC;

	// WidthやHeightはテクスチャの場合であれば画像の幅と高さを表す
	// 頂点の場合は、Widthが頂点数、Heightは１となる
	// SampleDescメンバーは、アンチエイリアシングを行うときのパラメータです。
	// 頂点の場合はアンチエイリアシングを行わないため、SampleDesc.Count = 1とする
	// 本来的には0としたいが、データがないことになってしまうため１としている
	// LayoutはUNKNOWNが適切に思えるが、MAJORとする
	// UNKNOWNと指定すると自動で最適なレイアウトを設定しようとするが、頂点の場合は不適切
	// MAJORはメモリが最初から終わりまでそのまま連続していることを示す

	// TITLE: 頂点バッファ－の生成
	// ID3D12Device::CreateCommittedResource()メソッドを呼び、リソースを作る
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);	// 頂点情報が入るだけのサイズ
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLGA_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	// 最後のID3D12Device::CreateCommittedResource()メソッドの戻り値にS_OKが戻ればＯＫ

	// TITLE: 頂点情報のコピー（マップ）
	// TITLE: ID3D12Resource::Map()メソッド
	// 頂点バッファ―を作りましたが、このバッファーに頂点情報をコピーしなければGPU側に頂点を渡すことができません。
	// ID3D12Resource::Map()メソッドを用いてバッファーに頂点情報をコピーしていきます。
	// ID3D12Resource::Map()メソッドとは、バッファーの（仮想）アドレスを取得するための関数です。
	// CPU側でこのアドレス上のメモリに対して変更を行えばそれがGPU側に伝わるイメージです。

	HRESULT Map(
		UINT Subresource,				// ミップマップなどではないため0でよい
		const D3D12_RANGE* pReadRange,	// 範囲指定。全範囲なのでnullptrでよい
		void** ppData					// 受け取るためのポインター変数のアドレス
	);

	// 第１引数は、リソース配列やミップマップの場合、サブリソース番号を渡す
	// 頂点バッファ―１つの場合は0
	// 第２引数のD3D12_RANGE構造体は、マップしたい範囲を特定するためのもの
	// バッファーの一部のみを更新したいときなどに設定しますが、今回は頂点すべてを更新するためnullptrとして全範囲を渡す
	// 第３引数は、ポインターのアドレス

	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	// マップしたvertMapに対して、事前に作っておいた頂点データを書き込んでやることによって、
	// バッファー上の頂点情報を更新する
	// なお、最後のID3D12Resource::Unmap()メソッドは「もうマップを解除してよい」という命令（アンマップ）

	// TITLE: 頂点バッファービューの作成
	// 頂点情報を利用するためには頂点バッファービュー（D3D12_VERTEX_BUFFER_VIEW構造体）が必要
	// これは「何バイトのデータが存在するのか」「１頂点あたり何バイトなのか」などを知らせるためのデータ

	typedef struct D3D12_VERTEX_BUFFER_VIEW
	{
		D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;	// 頂点バッファーのアドレス
		UINT SizeInBytes;							// 総バイト数
		UINT StrideInBytes;							// １頂点あたりのバイト数
	} D3D12_VERTEX_BUFFER_VIEW;

	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();	// バッファーの仮想アドレス
	vbView.SizeInBytes = sizeof(vertices);						// 全バイト数
	vbView.StrideInBytes = sizeof(vertices[0]);					// １頂点あたりのバイト数

	// ID3D12Resource::GetGPUVirtualAddress()メソッドは、GPU上のバッファーの仮想アドレスを取得するためのもの
	// アドレスを渡すことによって、GPU側はどのバッファーからデータをどのくらい見ればよいかわかる

	_cmdList->IASetVertexBuffers(0, 1, &vbView);

	// 上記により頂点バッファービューをGPUに伝える

	// TITLE: ID3DBlob型
	// シェーダーオブジェクトはID3DBlobという型のデータ
	// ID3DBlobはシェーダーオブジェクト以外にもさまざまな用途で使われる型であり、「汎用型」です
	// 実際にID3DBlobオブジェクトは「何かのデータの塊」を示すためのポインターとそのサイズを持っており、
	// それがどのようなデータなのかは使用する側が決定する

	// BLOBとは
	// BLOBとは、Binary Large Objectの略で、大きなデータの塊を指す言葉
	// 今回のシェーダーデータは大きくないですが、不定形のデータに対してこの名称が使われる
	// また、「blob」という英単にはもう１つ、「スライム」のようなドロドロしたゼリー状の不定形の物体という意味もある

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

	// TITLE:必要なインクルードとリンク指定
	// シェーダーのコンパイル機能に必要なヘッダーファイルと.libファイルのリンク指定を記述する必要があります

#include <d3dcompiler.h>
#pragma comment(lib, "d3dCompiler.lib")

	//TITLE: D3DCompileFromFile()関数
	// シェーダーファイルを読み込むための関数はD3DCompileFromFile()です

	HRESULT D3DCompileFromFile(
		LPCWSTR pFileName,					// ファイル名（ワイド文字）
		const D3D_SHADER_MACRO* pDefines,	// シェーダーマクロオブジェクト（nullptrでよい）
		ID3DInclude* pInclude,				// インクルードオブジェクト
		LPCSTR pEntrypoint,					// エントリポイント（呼び出すシェーダー名）
		LPCSTR pTarget,						// どのシェーダーを割り当てるか（vs、psなど）
		UINT Flags1,						// シェーダーコンパイルオプション
		UINT Flags2,						// エフェクトコンパイルオプション(0を推奨）
		ID3DBlob** ppCode,					// 受け取るためのポインターアドレス
		ID3DBlob** ppErrorMsgs				// エラー用ポインターのアドレス
	);

	// このうち重要なのは、第１引数のファイル名、第４引数のエントリポイント、第５引数のシェーダーターゲットです
	// 第１引数のpFileNameはファイル名ですが、ワイド文字列であることに注意する
	// ワイド文字列リテラルは、先頭にLを付けることで表せる

	LPCWSTR str = L"ワイド文字列";

	// 一方、他の引数であるエントリポイントや、ターゲットは「ワイドでない」文字列

	// char*やstd::stringからワイド文字列への変換
	// ワイド文字列リテラルを使わず、char*やstd::stringからワイド文字列に変換する方法は、後程

	// 第２引数のpDefinesは、今回使わないためnullptrを指定するが、C言語で言うところの#defineを配列で指定するための引数
	// 第３引数のpIncludeには、後ほどHLSLファイル内でインクルードを行うためにD3D_COMPILE_STANDARD_FILE_INCLUDEマクロを指定します
	// これにより、シェーダー中に#include文が書かれている場合、インクルードファイルのディレクトリとして、カレントディレクトリを参照するようになる
	// #include文を使わない場合はnullptrを指定する

	// カレントディレクトリ以外からインクルードする場合
	// もしカレントディレクトリ以外をインクルードディレクトリにする場合には、プログラマーが自前でID3DIncludeから継承したクラスを作り、
	// そのオブジェクトを渡す必要があります

	// 第４引数のpEntrypointは、エントリポイントです。
	// シェーダー内のどの関数を頂点シェーダーもしくはピクセルシェーダーのエントリポイントとして割り当てるのかを指定する
	// 第５引数のpTargetは、シェーダーの種類とバージョンを指定する文字列
	// 頂点シェーダーならvs、ピクセルシェーダーならpsとなり、そのあと_に続けてバージョンを指定する
	// 第６引数（Flag1）はシェーダーコンパイルオプションです。
	// コンパイルオプションは複数あり、それらをOR演算子（|）で組み合わせて指定する
	// 以下、一部
	// ・D3DCOMPILE_DEBUG：デバッグ用
	// ・D3DCOMPILE_SKIP_VALIDATION：シェーダーの検証を行わない
	// ・D3DCOMPILE_SKIP_OPTIMIZATION：シェーダーの最適化を行わない
	// ・D3DCOMPILE_PACK_MATRIX_ROW_MAJOR：行列は行優先とする
	// ・D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR：行列は列優先とする

	// そのほか、20種類以上のオプションがある

	// 第７引数のFlag2は、シェーダーファイルの場合0にすることが推奨されています。
	// 0以外の値にはD3DCOMPILE_EFFECT_CHILD_EFFECTとD3DCOMPILE_EFFECT_ALLOW_SLOW_OPSがあります
	// 最後の引数ppErrorMsgsは、エラーが起こったときに、エラー内容を受け取るポインターを指定する
	// エラーが起こらなければnullptrが返る

	result = D3DCompile(
		L"BasicVertexShader.hlsl",							// シェーダー名
		nullptr,											// defineなし
		D3D_COMPILE_STANDARD_FILE_INCLUDE,					// インクルードはデフォルト
		"BasicPS", "ps_5_0",								// 関数はBasicPS、対象シェーダーはps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	// デバッグ用及び最適化なし
		0,
		&_vsBlob, &errorBlob);								// エラー時はerrorBlobにメッセージが入る

	// エラーが起きた場合
	// 読み込み時にエラーが発生した場合は、たいていシェーダーの内容が間違っているのでまず確認する
	// シェーダーの内容が間違っている場合は、ppErrorMsgにシェーダーコンパイラーからの「かなり詳細な」メッセージが文字悦として入っている
	// ppErrorMsg内のメッセージを取り出すために、以下に示す２つのID3DBlobのメソッドを使う
	// ・ID3DBlob::GetBufferSize()：バッファーのサイズを返す（メッセージ文字列のバイト数）
	// ・ID3DBlob::GetBufferPointer()：バッファーのアドレスを返す（メッセージ文字列のアドレス）
	// これらを使えば、次のようにエラーメッセージをデバッグウィンドウに表示できます。
	// デバッグウィンドウへの表示はOutpuDebugStringA()関数を使用している

	std::string errstr;								// 受け取り用string
	errstr.resize(errorBlob->GetBufferSize());		// 必要なサイズを確保

	// データコピー
	std::copy_n(errstr.data(),
		errorBlob->GetBufferPointer(),
		errorBlob->GetBufferSize());

	OutputDebugStringA(errstr.c_str());	// データ表示

	// ファイル名間違い確認をチェックします
	// ファイル名が間違っている場合はHRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)が返ってくる
	// なお、この場合はppErrorMsgオブジェクトは何も入っていない(nullptr)
	// ファイル名を見直しましょう

	// 以下、頂点シェーダー読み込みからエラーチェックまで

	ID3Blob* errorBlob = nullptr;

	result = D3DCompileFromFile(L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &errorBlob);

	if (FAILED(result))
	{
		if (result == HERSULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見当たりません");
			return 0;	// exit()などに適宜置き換える方が良い
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());

			errstr += "\n";

			::OutputDebugStringA(errstr.c_str());
		}
	}

	//TITLE: 頂点レイアウト
	// 頂点レイアウトは頂点データをどう解釈するのかを表すもの
	// DirectX12では、頂点レイアウトはD3D12_INPUT_ELEMENT_DESC構造体の配列として定義する。

	//TITLE: D3D12_INPUT_ELEMENT_DESC構造体

	typedef struct D3D12_INPUT_ELEMENT_DESC
	{
		LPCSTR SemanticName;		// セマンティクス名
		UINT SemanticIndex;			// 同じセマンティクス名のときに使うインデックス（0でよい）
		DXGI_FORMAT Format;			// フォーマット（要素数とビット数で型を表す：後述）
		UINT InputSlot;				// 入力スロットインデックス（0でよい）
		UINT AlignedByteOffset;		// データのオフセット位置（D3D12_APPEND_ALIGNED_ELEMENTでよい）

		// INPUT_CLASSIFICATION_PER_VERTEX_DATAでよい
		D3D12_INPUT_CLASSIFICATION InputSlotClass;

		UINT InstanceDataStepRate;	// 一度に描画するインスタンスの数(0でよい）
	}D3D12_INPUT_ELEMENT_DESC;


	//TITLE: セマンティクス名：SemanticName
	// シェーダーのセマンティクス同様、データの意味を文字列で記述するもの。座標の場合は"POSITION"

	//TITLE: フォーマット（DXGI_FORMAT列挙型）
	// FormatにはDXGI_FORMAT列挙型を指定する
	// ・SINT：符号あり整数型（ビット数は32とは限らない）
	// ・UINT：符号なし整数型（ビット数は32とは限らない）
	// ・SNORM：-1.0f～1.0fに正規化された浮動小数点（ビット数は32とは限らない）
	// ・UNORM：0.0f～1.0fに正規化された浮動小数点（ビット数は32とは限らない）
	// ・TYPELESS：型なし（ここで型を特定せず、別の要因によって決まる）

	// TITLE: スロット関連のメンバー
	// スロットは、GPUが頂点データを見る「のぞき穴」のようなものだとイメージするとよいでしょう。
	// DirectX側でID3D12GraphicsCommandList::IASetVertexBuffers()メソッドを使ってスロットと実際のバッファーを関連付けます
	// 単純なケースの場合は１つでもよいですが、複数のスロットに分けてデータを扱うこともあります
	// スロット番号を指定するのがInputSlotメンバーです
	// 複数のスロットに頂点データが入力されるような仕様のとき、「このデータとこのデータはスロットnから」「このデータはスロットmから」と配置します
	// データを一度に流しこむことを「インターリープ」と呼びます
	// 場合によっては、「座標データ」と「法線」、「カラー」のようにスロットを分けることもあります。
	// データのバッファーを分けることを「非インターリープ」と呼びます

	// AlignedByteOffsetは、「そのデータの場所」を示します
	// 座標データのあとに法線データが来るとして、法線データの場所は32ビットのfloat３つ分のあとなので12バイトと指定します
	// 次から次にデータが並んでいる場合は、連続していることを表す定数D3D12_APPEND_ALIGNED_ELEMENTを指定します

	// InputSlotClassには、値としてD3D12_INPUT_CLASSFICATION_PER_VERTEX_DATAを指定する
	// この値は「データの内容として１頂点ごとにこのレイアウトが入っている」ということを表している
	// D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATAという列挙値があり、インスタンシングを使うときに指定する

	//TITLE: インスタンシング関連のメンバー
	// InstanceDataStepRateは、一度に描画するインスタンスの数を指定する
	// 今回は利用しないので0とする
	// もし複数のインスタンシングを行い、かつこのデータがインスタンスごとのデータである場合、このパラメータが２以上だと、そのデータはその数だけ同じデータを使いまわすことなる

	D3D12_INPUT_ELEMENT_DESC[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	//TITLE: グラフィックスパイプラインステートとは
	//DirectX12では、グラフィックスパイプレインの設定を定義するためにグラフィックスパイプラインステートというオブジェクト（ID3D12PipelineState*型）を使う。
	//グラフィックスパイプラインステートを作るにはID3D12Device::CreateGraphicsPipelineState()メソッドを使用する

	HRESULT CreateGraphicsPipelineState(
		const D3D12_GRAPHICS_PIPELINESTATE_DESC* pDesc, // 各パラメーター設定
		REFIID riid,									// IID_PPV_ARGSで設定
		void** ppPipelineState
	);

	// 最後の２引数、riidとppPipelineStateはIID_PPV_ARGSマクロを使います

	//TITLE: D3D12_GRAPHICS_PIPELINE_STATE_DESC構造体

	typedef struct D3D12_GRAPHICS_PIPELINE_STATE_DESC
	{
		ID3D12RootSignature* pRootSignature;	// ルートシグネチャ
		D3D12_SHADER_BYTECODE VS;	// 頂点シェーダー
		D3D12_SHADER_BYTECODE PS;	// ピクセルシェーダー
		D3D12_SHADER_BYTECODE DS;	// ドメインシェーダー
		D3D12_SHADER_BYTECODE HS;	// ハルシェーダー
		D3D12_SHADER_BYTECODE GS;	// ジオメトリシェーダー
		D3D12_STREAM_OUTPUT_DESC StreamOutput;	// ストリーミング出力バッファー設定
		D3D12_BLEND_DESC BlendState;	// ブレンドパラメーター
		UINT SampleMask;	// サンプルマスク
		D3D12_RASTERIZER_DESC RasterizerState;	// ラスタライザーの設定
		D3D12_DEPTH_STENCIL_DESC DepthStencilState;	// 深度設定
		D3D12_INPUT_LAYOUT_DESC InputLayout;	// 頂点レイアウト
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;	// ストリップ時のカット値設定

		// TOPOLOGY_TYPE_TRIANGLEでよい
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;

		UINT NumREnderTargets;					// レンダーターゲット数（１でよい）
		DXGI_FORMAT RTVFormats[8];				// 0番のみをR8G8B8A8_UNORMで設定
		DXGI_FORMAT DSVFormat;					// デプス値フォーマット
		DXGI_SAMPLE_DESC SampleDesc;			// マルチサンプリングの設定
		UINT NodeMask;							// アダプターは１個だけ使の宇野で0にする
		D3D12_CACHED_PIPELINE_STATE CachedPSO;	// 高速化に関係しているが今回は設定しない
		D3D12_PIPELINE_STATE_FLAGS Flags;		// ツールデバッグするかどうか
	}D3D12_GRAPHICS_PIPELINE_STATE_DESC;

	// 要約すると
	// ・頂点シェーダー：VS（必須）
	// ・ピクセルシェーダー：PS（必須）
	// ・ハルシェーダー：HS
	// ・ドメインシェーダー：DS
	// ・ジオメトリシェーダー：GS
	// ・ブレンドパラメーター：BlendState（必須）
	// ・ラスタライザーの設定：RasterrizerState（必須）
	// ・深度設定：DepthStencilState（必須）
	// ・頂点レイアウト：InputLayout（必須）
	// ・トポロジタイプ：PrimitiveTopologyType（必須）
	// ・ルートシグネチャ：pRootSignature（必須）

	//TITLE: シェーダーのセット
	// D3D12_SHADER_BYTECODE構造体は、メンバーとしてシェーダーのバイトコードのポインター（pShaderBytecode）と
	// サイズ情報（BytecodeLength）を持っています。
	// それらに、作成したID3D12Blobオブジェクトから取得したポインターとサイズを代入します

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipline = {};

	gpipeline.pRootSignature = nullptr;

	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	//TITLE: サンプルマスクとラスタライザーの設定

	// デフォルトのサンプルマスクを表す定数（0xffffffff）
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// まだアンチエイリアスは使わないためfalse
	gpipeline.RasterizerState.MultisampleEnable = false;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	// 中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable = true;			// 深度方向のクリッピングは有効

	// CullMode（D3D12_CULL_MODE列挙型）は背面カリングを行うかどうかを指定するためのメンバー
	// ポリゴンの中身を塗りつぶすために、FillMode（D3D12_FILL_MODE列挙型）にはD3D12_FILL_MODE_SOLIDを指定する
	// 深度方向のクリッピングは必要になるのでtrueにします

	//TITLE: ブレンドステートの設定
	// BLEND_DESC構造体には、すべてのレンダーターゲットに共通で設定する項目とレンダーターゲットごとに設定する項目とがあり、
	// レンダーターゲットごとの項目は、場合により「使用するレンダーターゲット分だけ」個別に設定しなければいけません。
	typedef struct D3D12_BLEND_DESC
	{
		BOOL AlphaToCoverageEnable;		// 今はfalseにする
		BOOL IndependentBlendEnable;	// 今はfalseにする
		D3D12_RENDER_TARGET_BLEND_DESC RenderTarget[8];	// レンダーターゲットごとの設定
	} D3D12_BLEND_DESC;

	// αテストとは、ラスタライザーが描画しようとしているピクセルのα値を調べて、
	// 「αが0であればそのピクセル描画自体を破棄する」ことによって描画対象から外すこと
	// これを行わないと、完全に透明な、描画する必要がないピクセルにも深度が書き込まれてしまうため、
	// たとえテクスチャの一部分が透明であってもそのポリゴンの向こうが見えなくなってしまうことになる
	// 手前に深度値が書き込まれると、より遠くにあるものが描画されない
	// 描画順にもよるが、もし手前が先に描画された場合、αテストなしの場合は向こう側のポリゴンが描画されなくなる
	// 処理負荷の面でも、描画する必要がないのにピクセルシェーダーの処理が走ってしまうのはもったいない
	// αが0でも、αテストではなく「αブレンディング」を行う場合、元の色と描画色をブレンドした色を取得し、
	// それを書き戻すのでそれなりのコストがかかります
	// これを回避するには、αテストではなく、ピクセルシェーダーに記述する必要があります

	if (color.a == 0)
	{
		discard;	// discardは対象ピクセルを破棄する（深度値も書き込まれない）
	}

	// ただしこれでは冗長なプログラムになるため、伝統的にαテストをオンにすることでこれらの問題を回避してきました。
	// しかし、DirectX12のパイプラインステートにはAlphaTestのようなメンバーはありません
	// 代わりにAlphaToCoverageEnableというメンバーが追加されています
	// マルチサンプリング（アンチエイリアシングの適用）でない場合AlphaToCoverageEnableの値がそのままαテストの有無を表すと考えてよいでしょう
	// 従来のAlphaTestとの違いは、マルチサンプリングによるアンチエイリアシングが適用された際に発揮されます
	// 従来のAlphatTestフラグでは「対象ピクセルのαかどうか」しか見ていなかったため、マルチサンプリング時に発生する「中間値のα」が抜かれずに
	// 汚い結果になってしまいました。
	// そのため、マルチサンプリング時に「網羅率」まで苦慮して描画を行うのがAlphaToCoverageEnableです
	// Coverage（網羅率）という単語からわかるとおり、アンチエイリアシングの結果、対象ピクセルの占有具合でαブレンディングを行います
	// 今回はαテストもアンチエイリアシングも行わないためfalseにしていますが、用途によってはtrueにしてもよいでしょう
	// IndependentBlendEnableにtrueを指定すれば、後述する合計８個のレンダーターゲット（RenderTarget[]）に対して
	// それぞれ別々のブレンドステートを割り当てられるようになります
	// falseに指定すると１つのブレンドステート（RenderTarget[0]）がすべてのレンダーターゲットに適用される

	typedef struct D3D12_RENDER_TARGET_BLEND_DESC
	{
		BOOL BlendEnable;				// 加算、乗算、αなどのブレンドするかどうか
		BOOL LogicOpEnable;				// 論理演算するかどうか
		D3D12_BLEND SrcBlend;			// 描画元係数
		D3D12_BLEND DestBlend;			// 描画先係数
		D3D12_BLEND_OP BlendOp;			// どうブレンドするのか
		D3D12_BLEND SrcBlendAlpha;		// 描画元α係数
		D3D12_BLEND_OP BlendOpAlpha;	// 描画先α係数
		D3D12_LOGIC_OP LogicOp;			// どう論理演算するのか
		UINT8 RenderTargetWriteMask;	// 書き込むときのマスク値
	}D3D12_RENDER_TARGET_BLEND_DESC;

	// 基本的には、レンダリング時の「描画」と「描画先」をどう合成するかという指定を行っている
	// まず、BlendOp（D3D12_BLEND_OP列挙体）やLogicOp（D3D12_LOGIC_OP列挙体）にあるOpとは、
	// Operator（演算）の略であり、これらのメンバーは「どう演算するのか」を表している
	// BlendEnableとLogicOpEnableは、「そもそも演算を行うかどうか」を表している
	// なお、これら２つのフラグは同時にtrueにすることができず、どちらかの演算を選ぶことになる
	// 現段階では特にブレンドを行わないため、BlendもLogicOpもfalseにする
	// RenderTargetWriteMaskはRGBAそれぞれの値を書き込むかどうか指定するための値
	// 例えば赤成分だけブレンドしたいならばD3D12_COLOR_WRITE_ENABLE_REDを指定する
	// 定数には RED / GREEN / BLUE / ALPHAがあり、それぞれ１ビットずつで表現されており、全体が４ビットで管理されている
	// すべての要素をブレンドしたければD3D12_COLOR_WRITE_ENABLE_ALLを指定する

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//TITLE: ブレンド処理の設定
	// 加減算などのブレンドを行う、つまりBlendOpEnableをtrueにする場合、BlendOpは以下の列挙値から選ぶことになる

	D3D12_BLEND_OP_ADD,				// 2つの値を加算
		D3D12_BLEND_OP_SUBTRACT,		// 2つの値を減算
		D3D12_BLEND_OP_REV_SUBSTRACT,	// 2つの値を減算（逆順）
		D3D12_BLEND_OP_MIN,				// 2つの値のうち小さい方
		D3D12_BLEND_OP_MAX,				// 2つの値のうち大きい方

		// 加算・減算だけで乗算がないのは、係数の設定と組み合わせることで実現できるため

		// LogicOpEnableをtrueにする（論理演算を行う）場合、LogicOpを以下の列挙値から選ぶことになる

		// すべてのビット０にする
		D3D12_LOGIC_OP_CLEAR,

		// すべてのビットを１にする
		D3D12_LOGIC_OP_SET,

		// 描画元の値をそのまま使用（Src）
		D3D12_LOGIC_OP_COPY,

		// 描画元の値を反転したものを使用（~Src）
		D3D12_LOGIC_OP_COPY_INVERTED,

		// 演算しない、つまり描画先のまま（Dst）
		D3D12_LOGIC_OP_NOOP,

		// ターゲット（描画先）を反転
		D3D12_LOGIC_OP_INVERT,

		// 描画元と描画先を論理積演算（Src & Dst）
		D3D12_LOGIC_OP_AND,

		// 描画元と描画先を否定論理積（論理積以外の部分）演算（~(Src & Dst)）
		D3D12_LOGIC_OP_NAND,

		// 描画元と描画先を論理和演算（Src | Dst）
		D3D12_LOGIC_OP_OR

		// 描画元と描画先を否定論理和（論理和以外の部分）演算（~(Src & Dst)）
		D3D12_LOGIC_OP_NOR,

		// 描画元と描画先を排他的論理和演算（Src ^ Dst）
		D3D12_LOGIC_OP_XOR,

		// 描画元と描画先が等しいビットは１、それ以外は０（~(Src ^ Dst)）
		D3D12_LOGIC_OP_EQUIV,

		// 描画先を反転したものと描画元を論理積演算（Src & ~Dst）
		D3D12_LOGIC_OP_AND_REVERSE,

		// 描画元を反転したものと描画先を論理積演算（~Src & Dst）
		D3D12_LOGIC_OP_AND_INVERTED,

		// 描画先を反転したものと描画元を論理和演算（Src | ~Dst）
		D3D12_LOGIC_OP_OR_REVERSE,

		// 描画元を反転したものと描画先を論理和演算（~Src | Dst）
		D3D12_LOGIC_OP_OR_INVERTED

		// BlendOpEnableとLogicOpEnableを同時にtrueにすることはできず、どちらかだけ、もしくは両方falseになる
		// LogicOpはビット演算であり、レンダーターゲットのフォーマットとしてはFLOATやUNORM/SNORMではなく、UINTを使用しなくてはならない

		//TITLE: ブレンドの係数
		// SrcBlend、DestBlend、SrcBlendAlpha、DestBlendAlphaについて
		// これらはさまざまな係数を指定するためのメンバーであり、ここではそれらの共通の型であるD3D12_BLEND列挙型について見てみる

		D3D12_BLEND_ZERO,			// 0乗算：何でもゼロになる
		D3D12_BLEND_ONE,			// 1乗算：素材の元の色になる
		D3D12_BLEND_SRC_COLOR,		// 描画元の色と乗算する
		D3D12_BLEND_INV_SRC_COLOR,	// 描画元の反転色と乗算する
		D3D12_BLEND_SRC_ALPHA,		// 描画元のα値と乗算する
		D3D12_BLEND_INV_SRC_ALPHA,	// 描画元のα値の反転値と乗算する
		D3D12_BLEND_DEST_ALPHA,		// 描画先のα値と乗算する
		D3D12_BLEND_INV_DEST_ALPHA,	// 描画先のα値の反転色と乗算する
		D3D12_BLEND_DEST_COLOR,		// 描画先の色と乗算する
		D3D12_BLEND_INV_DEST_COLOR,	// 描画先の色の反転値と乗算する

		// これらをBlendOpと組み合わせることで、さまざまなブレンディングが実現できる
		// 例として、αブレンディングについて考えてみる
		// αブレンディングとは、描画先に既に書き込まれている色（dest）と、描画元の「今から書き込む」ための色（src）を
		// α値をもとに合成すること

		// 合成結果 ＝ dest × (1 - α） ＋ src × α

		// これをBlendOpと係数の組み合わせで表現すると次のようになる
		// ・BlendOp：D3D12_BLEND_OP_ADD（加算）
		// ・SrcBlend：D3D12_BLEND_SRC_ALPHA（描画元のα値）
		// ・DestBlend：D3D12_BLEND_INV_SRC_ALPHA（描画元のα値の反転つまり1 - α）

		// 他にも加算合成は次のようになります
		// ・BlendOp：D3D12_BLEND_OP_ADD（加算）
		// ・SrcBlend：D3D12_BLEND_ONE（描画元そのままの値）
		// ・DestBlend：D3D12_BLEND_ONE（描画元そのままの値）

		// 乗算合成
		// ・BlendOp：D3D12_BLEND_OP_ADD（加算）
		// ・SrcBlend：D3D12_BLEND_ZERO（使用しない）
		// ・DestBlend：D3D12_BLEND_SRC_COLOR（描画元そのままの値）

		// 他の列挙値
		// 他にもD3D12_BLEND列挙型には以下のような列挙値があります
		D3D12_BLEND_SRC_ALPHA_SAT,		// αか1-αの小さい方を係数として採用
		D3D12_BLEND_BLEND_FACTOR,		// OMSetBlendFactor()関数から与えられる係数
		D3D12_BLEND_INV_BLEND_FACTOR,	// OMSetBlendFactor()関数から与えられる係数の反転値
		D3D12_BLEND_SRC1_COLOR,			// target1の色を係数とする
		D3D12_BLEND_INV_SRC1_COLOR,		// target1のα値を係数とする
		D3D12_BLEND_SRC1_ALPHA,			// target1のα値の反転値を係数とする

		// なおtarget1とは、シェーダー出力において、複数のターゲットを出力をしているときに１番に出力している値のこと
		// マルチレンダーターゲットを利用すると理解できます

		//TITLE: 入力レイアウトの設定
		//TITLE: InputLayout
		// 入力レイアウト（InputLayout：D3D12_INPUT_LAYOUT_DESC型）を設定する

		gpipeline.InputLayout.pInputElementDescs = inputLayout;		// レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);	// レイアウト配列の要素数

	//TITLE: _countofマクロ
	// _countofは、配列の要素数を取得するためのマクロであり、以下のコードと同じ意味
	gpipeline.InputLayout.NumElements = sizeof(inputLayout) / sizeof(inputLayout[0]);


	//TITLE: IBStripCutValue
	// 次は、IBStripCutValue（D3D12_INDEX_BUFFER_STRIP_CUT_VALUE列挙型）
	// 3Dオブジェクトは三角形の集合として表されますが、その表現方法には、トライアングルリストとトライアングルストリップという２つの方法が使われている
	// トライアングルリストとは、独立した三角形の集合体
	// トライアングルストリップは、「前の三角形とつながった三角形」の集合体
	// トライアングルストリップの方が頂点数が少なくて便利そうですが、実際は「次の頂点を１つ置くだけで勝手に三角形が構成されてしまう」など、
	// 柔軟性に乏しい方法とえいます。
	// そこで使われるのがIBStripCutValueで、このメンバーは、トライアングルストリップのときに、この「切り離せない頂点集合」を特定のインデックス
	// で切り離すための指定を行うためのものです。
	// 今回は、「特に切り離さない」ことを表すD3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLEを指定します

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLE;	// カットなし

	//TITLE: PrimitiveTopologyType
	// このメンバーは、構成要素が「点」「線」「三角形」のどれなのかを指定するためのもの（プリミティブトポロジ）です。
	// 三角形を表す場合はD3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//TITLE: レンダーターゲットの設定
	// １つ用意する場合は、NumRenderTargetsには1を指定します
	// レンダーターゲットのフォーマットを表すRTVFormats(DXGI_FORMAT列挙型の配列）も0番のみを使用する
	// RTVFormatsに指定する値は「0~1に正規化されたRGBA」を表すDXGI_FORMAT_R8G8B8A8_UNORMです

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8B8G8A8_UNORM;	// 0～1に正規化されたRGBA

	// 注意点として、レンダーターゲット数を１にしているのに、0番以外のレンダーターゲットを設定してはいけません。

	//TITLE: アンチエイリアシングのためのサンプル数設定
	// アンチエイリアシング（Anti-Aliasing：AA）は今回は設定しません
	gpipeline.SampleDesc.Count = 1;		// サンプリングは１ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;	// クオリティは最低

	//TITLE: グラフィックスパイプラインステートオブジェクトの生成
	// ID3D12PipelineStateオブジェクトを保持するポインター変数を宣言する

	ID3D12PipelineState* _pipelinestate = nullptr;

	// ID3D12Device::CreateGraphicsPipelineState()メソッドを呼び、オブジェクトを作成します

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	// これで実行すると、ルートシグネチャを実装していなければ、E_INVALIDARGエラーが返ってくる

	//TITLE: ルートシグネチャ
	// ルートシグネチャは「ディスクリプタテーブルをまとめたもの」であり、
	// 頂点情報以外のデータ（テクスチャや定数など）をグラフィックスパイプラインの外からシェーダーに送り込むために使われる

	// ディスクリプタテーブル
	// ディスクリプタテーブルは、ディスクリプタヒープ（テクスチャや定数などのアドレスを１か所にまとめておくためのヒープ領域）
	// とシェーダーのレジスターを関連付けるテーブルです

	// ルートシグネチャは、GPUに「どのスロットから何個のテクスチャ（や定数など）のデータを利用するのか」を教える役割を持っている
	// ルートシグネチャの「シグネチャ」とは、一言でいうと「パラメーターの型と並びを規定したもの」であり、シェーダーのパラメーター（レジスター）の並びをひとまとめにしたもの

	//TITLE: (空の)ルートシグネチャを作成する
	// ルートシグネチャがないとグラフィックスパイプラインステートは作れません
	// ルートシグネチャ情報が特に必要なくても「空のルートシグネチャ」を設定しなけれなりません
	// １．D3D12_ROOT_SIGNATURE_DESCオブジェクトに適切な設定を行う
	// ２．設定したD3D12_ROOT_SIGNATURE_DESCオブジェクトをもとに、ルートシグネチャのバイナリコード（ID3DBlob型）を作成する
	// ３．そのバイナリコードをもとに、ルートシグネチャオブジェクトを生成する

	//TITLE:D3D12_ROOT_SIGNATURE_DESCの設定
	typedef struct D3D12_ROOT_SIGNATURE_DESC
	{
		UINT NumParameters;
		const D3D12_ROOT_PARAMETER* pParameters;
		UINT NumStaticSamplers;
		const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers;
		D3D12_ROOT_SIGNATURE_FLAGS Flags;
	}D3D12_ROOT_SIGNATURE_DESC;

	// 空のルートシグネチャなので、ほぼ何も設定する必要はありません
	// ただし頂点情報だけは存在するので、Flagsに「頂点情報（入力アセンブラ）がある」という意味のD3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUTという列挙値を設定する
	D3D12_ROOT_SINGATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//TITLE:バイナリコードの作成
	// ルートシグネチャのバイナリコードを作成するにはD3D12SerializeRootSignature()関数を使用する
	ID3DBlob* rootSigBlobk = nullptr;

	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,				// ルートシグネチャ設定
		D3D_ROOT_SIGNATURE_VERSION_1_0,	// ルートシグネチャバージョン
		&rootSigBlob,					// シェーダーを作ったときと同じ
		&errorBlob);					// エラー処理も同じ

	// ルートシグネチャのバージョン
	D3D_ROOT_SIGNATURE_VERSION_1 = 0x1,
		D3D_ROOT_SIGNATURE_VERSION_1_0 = 0x1,
		D3D_ROOT_SIGNATURE_VERSION_1_1 = 0x2

		// バージョン1.1を利用する場合
		// ID3D12Device::CheckFeatureSupport()メソッドを使って、自分の環境で使えるかどうか確認する

		//TITLE: ルートシグネチャオブジェクトの作成
		// ID3D12Device::CreateRootSignature()メソッドを使用する
		result = _dev->CreateRootSignature(
			0,		// nodemask。0でよい
			rootSigBlob->GetBufferPointer(),	// シェーダーのときと同様
			rootSigBlob->GetBufferSize(),		// シェーダーのときと同様
			IID_PPV_ARGS(&rootsignature));

	rootSigBlob->Release();	// 不要になったので解放

	//TITLE: グラフィックスパイプラインステートに設定する
	// 生成したルートシグネチャオブジェクトを、グラフィックスパイプラインステートに設定する
	gpipeline.pRootSignature - rootsignature;

	// ID3D12Device::CreateGraphicsPipelineState()メソッドがS_OKを返せば成功

	//TITLE: ビューポートとシザー矩形
	//TITLE: ビューポートとは
	// ビューポートはDirectX 9にも存在している概念
	// 簡単に言うと、「画面（ウィンドウ）に対してレンダリング結果をどう表示するか」という設定
	// 設定するべきものは画面（ウィンドウ）のサイズと深度（デプス範囲）
	D3D12_VIEWPORT viewport = {};

	viewport.Width = window_width;		// 出力先の幅（ピクセル数）
	viewport.Height = window_height;	// 出力先の高さ（ピクセル数）
	viewport.TopLeftX = 0;				// 出力先の左上座標X
	viewport.TopLeftY = 0;				// 出力先の左上座標Y
	viewport.MaxDepth = 1.0f;			// 深度最大値
	viewport.MinDepth = 0.0f;			// 深度最小値

	// ビューポートを設定しただけではレンダリング結果を表示することはできず、
	// 一部だけを表示したい場合などもあるため、ビューポートと同時に「シザー矩形」というものを設定する

	//TITLE: シザー矩形
	// シザー矩形（Scissor Rectangle）とは、「ビューポートに出力された画像のどこからどこまでを実際に画面に映し出すか」を設定するためのもの
	// ビューポ―トに出力した画像を画面全体に表示したければ、シザー矩形も「ビューポートと同じ座標、同じ大きさ」にする
	// 一部だけであれば、ビューポートより小さく作る
	// 以下は画面全体に表示する設定

	D3D12_RECT scissorrect = {};

	scissorrect.top = 0;								// 切り抜き上座標
	scissorrect.left = 0;								// 切り抜き左座標
	scissorrect.right = scissorrect.left + window_width;	// 切り抜き左座標
	scissorrect.bottom = scissorrect.top + window_height;	// 切り抜き左座標

	// ビューポートとシザー矩形の使われ方
	// 作成したビューポートとシザー矩形は、描画命令時に
	// ID3D12GraphicsCommandList::RSSetViewports()メソッドと
	// ID3D12GraphicsCommandList::RSSetScissorRect()メソッドでGPU側に伝えることになる

	//TITLE: 描画命令
	// 「画面クリア」に追加する形で、命令をコマンドリストに積み重ねます
	// １．パイプラインステートのセット
	// ２．ルートシグネチャのセット
	// ３．ビューポートとシザー矩形のセット
	// ４．プリミティブトポロジのセット
	// ５．頂点情報のセット
	// ６．描画（Draw）命令

	// ID3D12GraphicsCommandList::SetPipelineState()メソッドでパイプラインステートを設定する

	_cmdList->SetPipelineState(_pipelinestate);

	// ID3D21GraphicsCommandList::SetGraphicsRootSignatrue()メソッドでルートシグネチャを設定する

	_cmdList->SetGraphicsRootSignature(rootsignature);

	// ID3D12GraphicsCommandList::IASetPrimitiveTopology()メソッドでプリミティブトポロジを設定する
	// プリミティブトポロジとは「頂点をどう組み合わせて、点・線・ポリゴンを構成するのか」という設定

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ID3D12GraphicsCommandList::IASetVertexBuffers()メソッドで頂点バッファーをセットする
	// 第１引数にはスロット番号を、第２引数には頂点バッファービューの数を、第３引数には頂点バッファービューの配列をセットする

	_cmdList->IASetVertexBuffers(0, 1, &vbView);

	// ID3D12GraphicsCommandList::DrawInstanced()メソッドで描画命令を設定する
	// 第１引数は頂点数、第２引数はインスタンス数、第３引数は頂点データのオフセット、第４引数はインスタンスのオフセット。

	_cmdList->DrawInstanced(3, 1, 0, 0);

	// インスタンス数
	// インスタンス数とは、「同じプリミティブ（ポリゴン）をいくつ表示するか」という意味

	// インデックスの実装
	// インデックスの準備
	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3,
	};

	// インデックスバッファーとしてGPUに転送する
	// ID3D12Device::CreateCommittedResource()メソッドで転送用のバッファーを作成する

	ID3D12Resource* idxBuff = nullptr;

	// 設定は、バッファーのサイズ以外、頂点バッファーの設定を使いまわしてよい
	resdesc.Width = sizeof(indices);

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	// マップしてデータをバッファーにコピーする

	// 作ったバッファーにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// インデックスバッファービューを作る
	// unsigned short(16ビット)でインデックス配列を使用するため、FormatメンバーはDXGI_FORMAT_R16_UINTとする

	// インデックスバッファービューを作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	// 描画時にID3D12GraphicsCommandList::IASetIndexBuffer()メソッドでインデックスバッファーのセット命令を設定する
	_cmdList->IASetIndexBuffer(&ibView);

	// インデックスバッファーは１つだけセットできる
	// 一度にセットできるインデックスバッファーは１つだけなので、配列要素はなく、そのままアドレスを割り当てている

	// ID3D12GraphicsCommandList::DrawInstanced()メソッドをID3D12GraphicsCommandList::DrawIndexedInstanced()メソッドに変更する

	_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// ID3D12GraphicsCommandList::DrawIndexedInstanced()メソッドでは、引数として頂点数ではなく描画対象となるインデックス数を指定する

	//TITLE: DirectX12におけるテクスチャ
	// DirectX12においてテクスチャは「グラフィックスパイプラインの流れの外から設定されるデータ」です。
	// テクスチャを扱うにあたり、まずGPU上のバッファーを確保し、ロードしたテクスチャをそのバッファーへとデータ転送する
	// という流れを意識しなければならない
	// テクスチャはプログラム上ではシェーダーリソースという言葉で示される

	//TITLE: ポリゴンにテクスチャを貼る手順
	// 「テクスチャをGPUバッファーに転送する」流れ
	// ・頂点情報にuv座標を追加する
	// ・頂点シェーダーを書き換える
	// ・ピクセルシェーダーを書き換える
	// ・CPU側のメモリ上にテクスチャ用データを作成する
	// ・GPUが利用できるよう転送先バッファーを作成する
	// ・CPU側にあるテクスチャデータをGPU側のバッファーへ転送する
	// ・ディスクリプタヒープを作成する
	// ・ディスクリプタヒープ上にシェーダーリソースビューを作る
	// ・ルートシグネチャのディスクリプタテーブルにテクスチャ用の設定（スロット番号範囲など）を書く
	// ・描画の際にディスクリプタヒープをセットする

	// DirectX11からのテクスチャ関連の変更点
	// DirectX12ではCPU側からテクスチャを設定する手順が大幅に変更された
	// DirectX11までであれば、D3DXライブラリのD3DXLoadTextureFromFile()関数や
	// DirectXTexライブラリのCreateWICTextureFlomFile()関数を呼ぶと、以下の処理を自動的に行ってくれました。
	// ・テクスチャ用バッファーの作成
	// ・バッファーへテクスチャデータを転送
	// ・シェーダーリソースビューの作成
	// ・ミップマップの作成
	// ・完了復帰（転送完了待ち）
	// DirectX12では、シェーダーリソースビューはディスクリプタヒープ上に作ることになっている
	// また、テクスチャを転送する方法も複数存在し、プログラマーが選択する

	//TITLE: 頂点情報にuv情報を追加する

	// 頂点情報
	XMFLOAT3 vertices[] =
	{
		{-0.4f, -0.7f, 0.0f},	// 左下
		{-0.4f,  0.7f, 0.0f},	// 左上
		{ 0.4f, -0.7f, 0.0f},	// 右下
		{ 0.4f,  0.7f, 0.0f},	// 右上
	};

	// これにuv情報（テクスチャ上の2D座標位置。それぞれ0.0f～1.0fで表される）を付加します
	// uv情報は2D座標であるため、floatが２つ分です。
	// しかし、頂点座標(float3つ)とuv座標(float2つ)を合わせて扱えるXMFLOAT5という型はないので構造体を作る

	// 頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos;	// xyz座標
		XMFLOAT2 uv;	// uv座標
	};

	// 頂点情報にuv座標を追加
	XMFLOAT3 vertices[] =
	{
		{{-0.4f, -0.7f, 0.0f},{0.0f, 1.0f}}	// 左下
		{{-0.4f,  0.7f, 0.0f},{0.0f, 0.0f}}	// 左上
		{{ 0.4f, -0.7f, 0.0f},{1.0f, 1.0f}}	// 右下
		{{ 0.4f,  0.7f, 0.0f},{1.0f, 0.0f}}	// 右上
	};

	Vertex* vertMap = nullptr;	// 型をVertexに変更

	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	//TITLE: シェーダーにuv情報を追加する
	// レイアウト設定およびシェーダーを書き換える
	// レイアウトに、uvを意味する"TEXCOORD"という項目を追加する
	D3D12_INPUT_ELEMENT_DEXC inputLayout[] =
	{
		{ // 座標情報
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv（追加）
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT ,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// ・32ビット
	// ・floatを表現する要素が2つ分

	// 設定したuv座標がシェーダー側に渡されることになるため、頂点シェーダーの引数にもfloat2 uv ： TEXCOORDを追加する

	BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)

	// uvが渡されているかどうかを確認するために、頂点シェーダーとピクセルシェーダーのやり取りを行うための構造体を用意する
	// この構造体は頂点シェーダーとピクセルシェーダーの両方で使われるため、BasicShaderHeader.hlsliというヘッダーファイルに記述して、
	// それぞれのHLSLファイルからインクルードする形にする

	#include "BasicShaderHeader.hlsli"

	// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
	struct Output
	{
		float4 svpos : SV_POSITION;	// システム用頂点座標
		float2 uv : TEXCOORD;		// uv値
	};

	// この型を頂点シェーダーの戻り値として使用すれば、ピクセルシェーダーの引数としてその値が渡されることになります
	
	Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
	{
		Output output;	// ピクセルシェーダーに渡す値
		output.svpos = pos;
		output.uv = uv;
		return output;
	}

	// ピクセルシェーダー側では引数としてこのOutput型のデータを受け取ります
	float4 BasicPS(Output input) : SV_TARGET
	{
		return float4(input.uv, 1, 1);
	}

	// uv情報が受け渡しされているかを確認するためにfloat4(input.uv,1,1);という値を返している
	// これはuv値をRGBAのRとGに割り当て、あとのBとAは1となるデータです
	// 左上が濃い青、右上が紫、左下が水色、右下が白となる

	//TITLE: テクスチャデータの作成
	// テクスチャデータをプログラムで生成してみる
	// レンダーターゲットがRGBAフォーマットなので、それに合わせたデータを作る
	// RGBAで幅が256、高さが256のデータを作る
	struct TexRGBA
	{
		unsigned char R, G, B, A;
	};

	std::vector texturedata(256 * 256);

	for (auto& rgba : textiredata)
	{
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;	// αは1.0とする
	}

	//TITLE: テクスチャバッファー
	// ID3D12Resourceオブジェクトとして作成する
	// 基本的には、頂点もインデックスも定数もGPU上のリソースはID3D12Resourceとして作る。
	// ID3D12Device::CreateCommittedResource()メソッドで作成する
	// 作成したテクスチャバッファーをCPUからGPUに転送するためには、次の２つの方法があります
	// ・ID3D12Resource::WriteToSubresource()メソッドを使う：比較的わかりやすいが特定の条件で効率が低下する
	// ・Id3D12Resource::Map()メソッドと、ID3D12GraphicsCommandList::CopyTextureRegion()/CopyBufferRegion()メソッドを利用する:推奨されているが難しい

	// TITLE:テクスチャバッファーの作成
	// ヒープの設定とリソースの設定を行う
	
	// ヒープの設定
	// WriteToSubresourceで転送するためのヒープ設定
	D3D12_HEAP_PROPERTIES heapprop = {};

	// 特殊な設定なのでDEFAULTでもUPLOADでもない
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;

	// ライトバック
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WIRTE_BACK;

	// 転送はL0、つまりCPU側から直接行う
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	// 単一アダプターのため0
	heapprop.CreationNodeMask = 0;
	heapprop.VisibleNodeMask = 0;


	// リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};

	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBAフォーマット
	resDesc.Width = 256;	// 幅
	resDesc.Height = 256;	// 高さ
	resDesc.DepthOrArraySize = 1;	// 2Dで配列でもないので1
	resDesc.SampleDesc.Count = 1;	// 通常テクスチャなのでアンチエイリアシングしない
	resDesc.SampleDesc.Quality = 0;	// クオリティは最低
	resDesc.MipLevels = 1;			// ミップマップしないのでミップ数は１つ
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2Dテクスチャ用
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;			// レイアウトは決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;				// 特にフラグなし

	// リソースの生成
	ID3D12Resource* texbuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,	// テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff));

	//TITLE: ID3D12Resource::WriteToSubresource()メソッドによるデータ転送
	HRESULT WriteToSubresource(
		UINT DstSubresource,		// サブリソースインデックス
		const D3D12_BOX* pDstBox,	// 書き込み領域の指定(nullptrならば先頭から全領域となる)
		const void* pSrcData,		// 書き込みたいデータのアドレス
		UINT SrcRowPitch,			// 1行あたりのデータサイズ
		UINT SrcDepthPitch			// スライスあたりのデータサイズ
	);

	// 3Dテクスチャや2Dテクスチャの場合、最後の引数のスライスは、配列の要素のことを指す。

	result = texbuffe->WriteToSubresource(
		0,
		nullptr,								// 全領域へコピー
		texturedata.data(),						// 元データアドレス
		sizeof(TexRGBA) * 256,					// 1ラインサイズ
		sizeof(TexRGBA) * texturedata.size()	// 全サイズ
	);


	//TITLE: シェーダーリソースビュー
	// テクスチャや定数バッファーに対するビューは「ディスクリプタヒープ」上で作らなければならない

	//TITLE: ディスクリプタヒープを作る
	// D3D12_DESCRIPTOR_HEAP_DESC構造体とID3D12Device::CreateDescriptorHeap()メソッドを使用する

	D3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

	// シェーダーから見えるように
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// マスクは0
	descHeapDesc.NodeMask = 0;

	// ビューは今のところ１つだけ
	descHeapDesc.NumDescriptors = 1;

	// シェーダーリソースビュー用
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	// 生成
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));


	//TITLE: シェーダーリソースビューを作る
	// D3D12_SHADER_RESOURCE_VIEW_DESC構造体で指定する
	// 基本的には「共用体」と「どのメンバーを使うか」のペアになっている。

	typedef struct D3D12_SHADER_RESOURCE_VIEW_DESC
	{
		// テクスチャのフォーマット
		DXGI_FORMAT Format;

		// 共用体のどのメンバーとして扱うか
		D3D12_SRV_DIMENSION ViewDimension;

		// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPINGでよい
		UINT Shader4ComponentMapping;

		union
		{
			D3D12_BUFFER_SRV Buffer;
			D3D12_TEX1D_SRV Texture1D;
			D3D12_TEX1D_ARRAY_SRV Texture1Darray;
			D3D12_TEX2D_SRV Texture2D;	// このメンバーを使用
			D3D12_TEX2D_ARRAY_SRV Texture2DArray;
			D3D12_TEX2DMS_SRV Texture2DMS;
			D3D12_TEX2DMS_ARRAY_SRV Texture2DMSArray;
			D3D12_TEX3D_SRV Texture3D;
			D3D12_TEXCUBE_SRV TextureCube;
			D3D12_TEXCUBE_ARRAY_SRV TextureCubeArray;
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV RaytracingAccelerationStructure;
		};
	}D3D12_SHADER_RESOURCE_VIEW_DESC;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBA(0.0f～1.0fに正規化)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;	// ミップマップは使用しないので１

	// Shader4ComponentMappingメンバーは、データのRGBAをどのようにマッピングするかということを指定するためのもの
	// D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPINGはマクロとして定義されており、画像の情報がそのままRGBAなど
	// 「指定されたフォーマットに、データどおりの順序で割り当てられている」ことを表している
	// シェーダーリソースビューの生成には、ID3D12Device::CreateShaderResourceView()メソッドを使用する

	_dev->CreateShaderResourceView(
		texbuff,	// ビューと関連付けるバッファー
		&srcDesc,	// 先ほど設定したテクスチャ設定情報
		texDescHeap->GetCPUDescriptorHandleForHeapStart()	// ヒープのどこに割り当てるか
	);

	// 最後の引数は、ディスクリプタヒープのどこにこのビューを配置するかを指定するためのもの
	// テクスチャビューが１つの場合、先頭、つまりID3D12DescriptorHeap::GetCPUDescriptorHandleForHeapStart()メソッドで取得した
	// 「アドレスのようなもの」であるハンドル（D3D12_CPU_DESCRIPTOR_HANDLE型）を指定する
	// 複数ある場合は、取得したハンドルからのオフセットを指定する必要がある

	//TITLE: ルートシグネチャにスロットとテクスチャの関連を記述する
	//TITLE: ディスクリプタテーブルとは
	// ディスクリプタテーブルは、ルートシグネチャを作成する際に設定する項目
	// ディスクリプタテーブルは、シェーダーレジスターと関連した概念
	// 定数バッファーやテクスチャなどをCPUとGPUでやり取りする際、レジスター種別とレジスター番号をリソースに割り当てて、
	// それをCPUとGPUの共通認識としてやり取りする。
	// その指定のセットをまとめているものがディスクリプタテーブル
	// DirectX12では、ディスクリプタテーブルの実体はルートパラメーター（D3D12_ROOT_PARAMETER構造体）というデータ
	// ルートパラメーターを作成後、ルートシグネチャの作成時にD3D12_ROOT_SIGNATURE_DESCオブジェクトにそのルートパラメーターを設定して利用する
	
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	rootSignatureDesc.pParameters = (/*作成するルートパラメータ配列の先頭アドレス*/);
	rootSignatureDesc.NumParameters = 1;

	// pParametersメンバーにはルートパラメ－ター配列の先頭アドレスを、NumParametersメンバーにはルートパラメーター数を設定する

	//TITLE: ルートパラメーターの作成
	typedef struct D3D12_ROOT_PARAMETER
	{
		D3D12_ROOT_PARAMETER_TYPE ParameterType;	// ルートパラメータータイプ
		union
		{
			D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;	// これを使用
			D3D12_ROOT_CONSTANTS Constants;
			D3D12_ROOT_DESCRIPTOR Descriptor;
		};
		D3D12_SHADER_VISIBILITY ShaderVisibility;	// どのシェーダーから利用可能か
	}D3D12_ROOT_PARAMETER;

	// 共用体なので、ParameterTypeメンバーに指定した種別によってメモリの解釈が変わる
	// ShaderVisibility（D3D12_SHADER_VISIBILITY型）には「ピクセルシェーダーから利用可能」ということを表すD3D12_SHADER_VISIBILITY_PIXELを指定する
	
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D1_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	// ピクセルシェーダーから見える
	rootparame.ShaderVisibility = D3D12_SHADAER_VISIBILITY_PIXEL;

	// DescriptorTableメンバーの型であるD3D12_ROOT_DESCRIPTOR_TABLE構造体
	typedef struct D3D12_ROOT_DESCRIPTOR_TABLE
	{
		// ディスクリプタレンジ数
		UINT NumDescriptorRanges;

		// ディスクリプタレンジのアドレス
		const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges;
	} D3D12_ROOT_DESCRIPTOR_TABLE;

	//TITLE:ディスクリプタレンジ
	// ディスクリプタヒープ上に種類が同じディスクリプタが連続して存在している場合には、それらをまとめて使用できるようにディスクリプタレンジを使って指定する
	typedef struct D3D12_DESCRIPTOR_RANGE
	{
		// レンジ種別（SRV）
		D3D12_DESCRIPTOR_RANGE_TYPE RangeType;

		// ディスクリプタ数（今回は１）
		UINT NumDescriptors;

		// 先頭レジスター番号
		UINT BaseShaderRegister;

		// つじつまを合わせるためのスペース（0でよい）
		UINT RegisterSpace;

		// D3D12_DESCRIPTOR_RANGE_OFFSET_APPENDでよい
		UINT OffsetInDescriptorFromTableStart;
	}D3D12_DESCRIPTOR_RANGE;

	// ディスクリプタ数が１の場合、NumDescriptorsメンバーには１を指定する
	// 「複数のテクスチャがディスクリプタヒープ上で並んでおり、連続して指定する場合」はこの数が増えます
	// OffsetInDescriptorsFromTableStartメンバーは「連続したディスクリプタレンジの直後に来る」という意味のD3D12_DESCRIPTOR_RANGE_OFFSET_APPENDを指定する

	D3D12_DESCRIPTR_RANGE descTblRange = {};

	descTblRange.NumDescriptrs = 1;
	descTblRange.RangeType = D3D12_DESCRIPTR_RANGE_TYPE_SRV;	// 種別はテクスチャ
	descTblRange.BaseShaderRegister = 0;	// 0番スロットから
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメーターの定義
	D3D12_ROOT_PARAMETER rootparam = {};

	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	// ピクセルシェーダーから見える
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ディスクリプタレンジのアドレス
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;

	// ディスクリプタレンジ数
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	// ルートシグネチャにルートパラメーターの追加
	rootSignatureDesc.pParameters = &rootparam;	// ルートパラメーターの先頭アドレス
	rootSignatureDesc.NumParameters = 1;		// ルートパラメーター数

	//TITLE: サンプラ―の設定
	// サンプラ―とは、uv値によってテクスチャデータからどのように色を取り出すかを決めるための設定
	// 「アドレッシングモード」とは、テクスチャのuv値が0～1の範囲外の場合の挙動を規定したもの
	typedef struct D3D12_STATIC_SAMPLER_DESC
	{
		D3D12_FILTER Filter;						// どう補完するか
		D3D12_TEXTURE_ADDRESS_MODE AddressU;		// u方向アドレッシングモード（繰り返し）
		D3D12_TEXTURE_ADDRESS_MODE AddressV;		// v方向アドレッシングモード（繰り返し）
		D3D12_TEXTURE_ADDRESS_MODE AddressW;		// w方向アドレッシングモード（繰り返し）
		FLOAT MipLODBias;							// 計算結果ミップマップレベルからのオフセット
		UINT MaxAnisotropy;							// 異方性フィルターの最大値
		D3D12_COMPARISON_FUNC ComparisonFunc;		// 比較関数
		D3D12_STATIC_BORDER_COLOR BorderColor;		// 端の色
		FLOAT MinLOD;								// ミップマップ下限
		FLOAT MaxLOD;								// ミップマップ上限
		UINT ShaderRegister;						// スロット番号
		UINT RegisterSpace;							// レジスタースペース（０でよい）
		D3D12_SHADER_VISIBILITY ShaderVisibility;	// どのシェーダーから参照可能か
	}D3D12_STATIC_SAMPLER_DESC;

	// w方向とは
	// これは3Dテクスチャにおいて利用されるアドレッシングモード
	// 3Dテクスチャにおいては、wが奥行き方向を表す

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// 横方向の繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// 縦方向の繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// 縦方向の繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	// ボーダーは黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// 線形補間
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;	// ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;				// ミップマップ最小値
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// ピクセルシェーダーから見える
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	// リサンプリングしない
	
	// ルートシグネチャに設定する
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//TITLE: 描画時の設定
	// ・ルートシグネチャの指定
	// ・ディスクリプタヒープの指定
	// ・ルートパラメーターインデックスとディスクリプタヒープのアドレスの関連付け

	//TITLE: ルートシグネチャの指定
	// ID3D12GraphicsCommandList::SetGraphicsRootSignature()メソッドを呼び出す
	_cmdList->SetFGraphicsRootSignature(rootSignatureDesc);

	//TITLE: ディスクリプタヒープの指定
	// ID3D12GraphicsCommandList::SetDescriptorHeaps()メソッドに、ディスクリプタヒープ数とディスクリプタヒープ配列を指定する
	_cmdList->SetDescriptorHeaps(1, &texDescHeap);

	//TITLE: ルートパラメーターとディスクリプタヒープの関連付け
	// ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable()メソッドでルートパラメーターインデックスと
	// ディスクリプタヒープのアドレス（ハンドル）を関連付ける
	
	_cmdList->SetGraphicsRootDescriptorTable(
		0,	// ルートパラメーターインデックス
		texDescHeap->GetGPUDescriptorHandleForHeapStart());	// ヒープアドレス

	//TITLE: ピクセルシェーダ―のプログラムを変更
	// テクスチャを表示できるようにピクセルシェーダーを変更する
	// テクスチャを参照するために、レジスターから情報を取得できるようにする
	// BasicShaderHeader.hlsliにスロットに関連付けられている情報に変数名を付けて使用できるように記述する

	texture2D<float4> tex: register(t0);	// 0番スロットに設定されたテクスチャ
	samperState smp : register(s0);			// 0番スロットに設定されたサンプラー

	float4 BasicPS(Output input) : SV_TARGET
	{
		return float4(tex.Sample(smp,input.uv));
	}

	// サンプラーを線形補間（バイリニア）にしているので、ノイズはぼやけます。
	// くっきりさせる場合は、ポイントサンプリングに変更する
	// 補間しない（ニアレストネイバー法：最近傍補間（さいきんぼうほかん）
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	// ドット絵や、ノイズの場合は補完しない方がよいが、通常のテクスチャはバイリニアで補間するとよい
	
	// TITLE: 画像ファイルを読み込んで表示する

	//TITLE: DirectXTexの準備
	// git clone もしくはZIPファイルとしてDL
	//https://github.com/Microsoft/DirectXTex

	// DirectXTex利用時の注意
	// Windows SDKのバージョンに注意する
	// ReadMe.txtに指定されたバージョンでない場合、正しく動作しない場合がある

	// ソリューションファイルを開き、DirectXTexをビルドする
	// 利用するプロジェクトに合わせて
	// ・ソリューション プラットフォーム：Win32 / x64
	// ・ソリューション構成：Debug / Release
	// を選択する

	//TITLE: 画像ファイルのロード
	// 基本的なファイル形式のロードはLoadFromWICFile()関数を使う
	//TDAやDDSなどのファイル形式はこれ以外の関数を使うことになる

	HRESULT LoadFromWICFile(
		const wchar_t* szFile,
		DWORD flags,
		TexMetadata* metadata,
		ScratchImage& image);

	// DirectXTexライブラリはオープンソースライブラリ

	// 第１引数はファイルパス
	// ワイド文字列（wchar_t*型）
	// もし文字列リテラルならば、L"ファイル名.png"のように文字列の前にLを付けて定義する
	// 第２引数はどのようにロードするかを示すフラグ
	// 読み込んだデータをそのまま使うには、「特別なことはしない」という意味のWIC_FLAGS_NONEだけを使う
	// 第３引数はメタデータ（TexMetadata型）を受け取るためのポインター
	// メタデータとは、画像ファイルに関する情報（幅・高さ・フォーマットなど）であり、これを参照してロードした画像データの解釈に利用することができる
	// 第４引数では、ScratchImage型の参照を渡す
	// 関数を実行すると、このオブジェクトに実際のデータが入る
	// ScratchImage::GetImage()メソッドで生画像データ（Image型）を取得して、それをID3D12Resource::WriteToSubresource()でGPUに転送する

	// なお、DirectXTexライブラリとDirectXMathライブラリは、DIrectXという名前空間を使用している
	
	// インターフェイス関連のエラーが出る場合
	// もしLoadFromWICFile()関数の実行時にインターフェイス関連のエラーメッセージが出る場合は、

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	// を最初に記述する
	// LoadFromWICFile()関数は、Windowsが用意する機能を使用してロードを行うが、その際にCOMを使うため、初期化する必要がある

	// ScratchImageオブジェクトから生の画像データ（Image型）を取り出す
	// ScratchImage::GetImage()メソッドを使う

	const Image* ScratchImage::GetImage(
		size_t mip,
		size_t item,
		size_t slice) const;

	// 第１引数はミップレベルです。
	// 第２引数には、テクスチャ配列を使用する際にインデックスを指定する
	// 第３引数にはスライス、つまり３Ｄテクスチャにおける深さを指定する

	// WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	result = LoadFromWICFile(
		L"img/textest.png",
		WIC_FLAGS_NONE,
		&metadata,
		scratchImg);

	auto img = scratchImg.GetImage(0, 0, 0);	// 生データ抽出

	struct Image
	{
		size_t width;		// 画像の幅
		size_t height;		// 画像の高さ
		DXGI_FORMAT format;	// 画像データフォーマット
		size_t rowPitch;	// 1行のデータサイズ
		size_t slicePitch;	// 1枚のデータサイズ
		uint8_t* pixels;	// 生データアドレス
	};

	// メタデータとの重複も多いが、ミップレベル数やテクスチャ配列のサイズなどの情報はメタデータ側にしかない

	//TITLE: 画像に合わせた設定
	// D3D12_RESOURCE_DESCオブジェクトを書き換えて、画像データ用にする
	
	resDesc.Width = metadata.width;		// 幅
	resDesc.Height = metadata.height;	// 高さ
	resDesc.DepthOrArraySize = metadata.arryaSize;
	resDesc.MipLevels = metadata.mipLevels;
	resDesc.Dimension - static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

	// Dimensionメンバーは、同じ数値で型名が違う列挙値であるためキャストしている（DirectX10～12で共用しているライブラリのため）

	//TITLE: 画像データをGPUに転送する
	
	result = texbuff->WriteToSubresource(
		0,
		nullptr,		// 全領域へコピー
		img->pixels,	// 元データアドレス
		img->rowPitch,	// 1ラインサイズ
		img->slicePitch	// 1枚サイズ
	);

	//TITLE: 画像データのフォーマットに合わせる
	// 画像のフォーマットに合わせて、リソースのFormatメンバーを変更する
	resDesc.Fromat = metadata.format;

	// リソースのフォーマットを変更すると、シェーダーリソースビューのFormatメンバーも同時に変更しないとビューの生成に失敗する
	srvDesc.Format = metadata.format;

	//TITLE: ガンマ補正に対応する
	// 画像フォーマットによっては、元の絵よりも暗く見えたり、コンストラストが上がって見えたりする
	// ガンマ補正値と呼ばれるものが関係しており、元画像がsRGBという色空間に対応したフォーマットである場合、
	// ガンマ補正なしのRGBAレンダーターゲットに表示しようとすると、このような問題が発生する
	// ガンマ補正を行って正しい色を出したい場合は、レンダーターゲットビューを修正する
	// なお、そのときもスワップチェーンのフォーマットはsRGBにしてはいけない
	// スワップチェーン生成が失敗してしまいます
	// あくまでもレンダーターゲットビューだけです
	//IDrectX12Device::CreateRenderTargetView()メソッドの第２引数がnullptrになっていると、「スワップチェーンに準拠する」という意味になっている
	
	for (int i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));
		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);	// ここを変更する
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// sRGB用のレンダーターゲットビュー設定を作る
	// sRGB用のレンダーターゲットビュー設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// ガンマ補正あり（sRGB）
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// これをIDirectX12Device::CreateRenderTargetView()メソッドの第２引数に渡す
	_dev->CreateRenderTargetView(_backBuffers[i], &rtvDesc, handle);

	// このやり方の場合、バックバッファーフォーマットとレンダーターゲットフォーマットに食い違いが生じるためデバッグレイヤーをオンにしているとエラーが表示される

	//パイプラインの設定
	// ビューをDXGI_FORMAT_R8G8B8A8_UNORM_SRGBにしているため、パイプラインのフォーマット（gpipeline.RTVFormats[0]）の方は変えても変えなくても
	// 出力に差はありません。ただし、パイプラインのみをDXGI_FORMAT_R8G8B8A8_UNORM_SRGBにしてしまうと、ガンマ補正がかからないうえにエラーを出力します

	//TITLE: ID3D12GraphicsCommandList::CopyTextureRegion()メソッドによる転送
	// ID3D12Resource::WriteToSubresource()メソッドを使った転送をおっこないましたが、
	// 特にディスクリートGPU（グラフィックスボードが別挿しなど）のときに、ID3D12Resource::WriteToSubresource()メソッドでは転送がうまくいかない
	// もしくは動作が遅くなることがあります。
	// CPUからGPUへの転送の際、ID3D12Resource::WriteToSubresource()メソッドはCPU主導でGPUへ転送するためUMA(Unified Memory Access)の際には
	// 優位性を発揮するものの、DMA(Direct Memory Access)での転送には向いていないためです

	// キャッシュの場所による違い
	// キャッシュをCPU側に取る(D3D12_HEAP_TYPE_UPLOAD)か、GPU側に取る(D3D12_HEAP_TYPE_DEFAULT)かの違いでこの差が生まれる

	// そこで対応として、まずアップロード用のリソースをGPU上に作り、テクスチャのための読み出し用メモリへのコピーを行います。
	// アップロード用のリソースを作成する点は、頂点バッファーなどと同様です
	// ただそうして作成したリソースをそのままテクスチャとして使用できれば良いのですが、テクスチャとして使用するには読み出し用リソースである
	// 必要があるため、「CPU側のテクスチャデータ」をシェーダーリソースとして扱える状態にしなければなりません。

	// アップロード用のリソースはあくまでも中間バッファーであり、テクスチャ用のものではありません
	// フォーマットにRGBAなどを指定してはいけません
	// 単なる連続したメモリとして指定する(DXGI_FORMAT_UNKNOWN)

	// それから、テクスチャのための読み出し用メモリへのコピーを行う
	// ID3D12GraphicsCommandList::CopyTextureRegion()メソッドを使う
	
	// １．アップロード用リソースの作成
	// ２．読み出し用リソースの作成
	// ３．アップロード用リソースへテクスチャデータをID3D12Resource::Map()メソッドでコピー
	// ４．アップロード用リソースから読み出し用リソースへID3D12GraphicsCommandList::CopyTextureRegion()メソッドでコピー

	// となる。
	// これだけの手順を踏むことでようやく、シェーダーリソースとしてテクスチャが参照可能になります
	// グラフィックスボード内部のメモリ（ＶＲＡＭ）にはさまざまな種類がありますが、大きく「CPUアクセス特化型」と「GPUアクセス特化型」に分けられます

	// CPUアクセス特化型はMap()メソッドでコピー可能ですが非常に遅いため、頻繁にデータ参照されるテクスチャとして利用するのは不向きです

	//TITLE: アップロード用リソースの作成
	// アップロード用リソースのヒープの設定
	
	// 中間バッファーとしてのアップロードヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};

	// マップ可能にするため、UPLOADにする
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	// アップロード用に使用すること前提なのでUNKNOWNでよい
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	uploadHeapProp.CreationNodeMask = 0;	// 単一アダプターのため0
	uploadHeapProp.VisibleNodeMask = 0;		// 単一アダプターのため0

	// リソース設定
	D3D12_RESOURCE_DESC resDesc = {};

	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Dimension - D3D12_RESOURCE_DIMENSION_BUFFER;	// 単なるバッファーとして指定

	resDesc.Width = img->slicePitch;	// データサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;

	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// 連続したデータ
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;			// 特にフラグなし

	resDesc.SampleDesc.Count = 1;	// 通常テクスチャなのでアンチエイリアシングしない
	resDesc.SampleDesc.Quality = 0;


	// 中間バッファー作成
	ID3D12Resource* uploadbuff = nullptr;

	result = _dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,	// 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	// CPUからマップ（Map()メソッドでコピー）するため、リソースステートをD3D12_RESOURCE_STATE_GENERIC_READ「CPUから書き込み可能だが、GPUから見ると読み取りのみ」にする

	//TITLE: コピー先リソースの作成
	// ヒープの設定
	// テクスチャのためのヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;	// テクスチャ用
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;	// 単一アダプターのため0
	texHeapProp.VisibleNodeMask = 0;	// 単一アダプターのため0

	// リソース設定
	resDesc.Format = metadata.format;
	resDesc.Width = metadata.width;	// 幅
	resDesc.Height = metadata.height;	// 高さ
	resDesc.DepthOrArraySize = metadata.arraySize;	// 2Dで配列でもないので１
	resDesc.MipLevels = metadata.mipLevels;	// ミップマップしないのでミップ数は１つ
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	// テクスチャバッファーの作成
	ID3D12Resource* texbuff = nullptr;

	result = _dev->CreateCommittedResource(
		&texheapProp,
		D3D12_HEAP_FLAG_NONE, // 特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,	// コピー先
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	//TITLE: アップロードリソースへのマップ

	uint8_t* mapforImg = nullptr;	// image->pixelsと同じ型にする
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg);	// マップ
	std::copy_n(img->pixels, img->slicePitch, mapforImg);	// コピー
	uploadbuff->Unmap(0, nullptr);	// アンマップ

	//TITLE: ID3D12GraphicsCommandList::CopyTextureRegion()メソッド
	// ID3D12GraphicsCommandList::CopyTextureRegion()メソッドはコマンドリストのメソッドです。
	// コピー元もコピー先もグラフィックスボード上なのでGPUに対する命令となります。

	void CopyTectureRegion(
		const D3D12_TEXTURE_COPY_LOCATION* pDst,	// グラフィックスボード上のコピー先アドレス
		UINT DstX,	// コピー先領域開始X（0でよい）
		UINT DstY,	// コピー先領域開始Y（0でよい）
		UINT DstZ,	// コピー先領域開始Z（0でよい）
		const D3D12_TEXTURE_COPY_LOCATION* pSrc,	// グラフィックスボード上のコピー元アドレス
		const D3D12_BOX* pSrcBox	// コピー元領域ボックス（nullptrでよい）
	);

	// D3D12_TEXTURE_COPY_LOCATION構造体
	typedef struct D3D12_TEXTURE_COPY_LOCATION
	{
		ID3D12Resource* pResource;	// リソースを入れる
		D3D12_TEXTURE_COPY_TYPE Type;	// コピー種別（FOOTPRINTかINDEXを選ぶ）
		union // 共用体
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;	// コピー領域に関する情報
			UINT SubresourceIndex;	// インデックス
		};
	}D3D12_TEXTURE_COPY_LOCATION;

	// Footprint（フットプリント）とは「メモリ占有領域に関する情報」という意味

	// D3D12_PLACED_SUBRESOURCE_FOOTPRINT構造体
	typedef struct D3D12_PLACED_SUBRESOURCE_FOOTPRINT
	{
		UINT64 Offset;	// 0でよい
		D3D12_SUBRESOURCE_FOOTPRINT Footprint;
	}D3D12_PLACED_SUBRESOURCE_FOOTPRINT;

	// 
	typedef struct D3D12_SUBRESOURCE_FOOTPRINT
	{
		DXGI_FORMAT Format;	// フォーマット
		UINT Width;		// 幅
		UINT Height;	// 高さ
		UINT Depth;		// 深さ
		UINT RowPitch;	// １行あたりのバイト数
	}D3D12_SUBRESOURCE_FOOTPRINT;


	// 「TypeがD3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINTの場合、pResourceはバッファーリソースを指す必要があります。」
	// 「TypeがD3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEXの場合、pResourceはテクスチャリソースを指す必要があります。」

	// アップロードバッファーにはフットプリントを指定し、テクスチャバッファーにはインデックスを指定しなければならない
	
	D3D12_TEXTURE_COPY_LOCATION src = {};

	// コピー元（アップロード側）設定
	src.pResource = uploadbuff;	// 中間バッファー
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;	// フットプリント指定
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metadata.width;
	src.PlacedFootprint.Footprint.Height = metadata.height;
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = img->rowPitch;
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	// コピー先設定
	dst.pResource = texbuff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);


	//TITLE: バリアとフェンスの設定
	// コピ－操作はGPUに対する命令のため、フェンスを用いて待ちを入れないといけません。
	// また、テクスチャ用リソースが「コピー先」のままなので、バリアを用いて前後の状態をCOPY_DESTとPIXEL_SHADER_RESOURCEにする必要がある

	D3D12_RESOURCE_BARRIER BarrierDesc = {};

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = texbuff;
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;	// ここが重要
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	// ここも重要

	// ID3D12CommandQueue::ExecuteCommandLists()メソッドで転送を実行する
	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();

	// コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	// フェンスで待ちを入れる
	_cmdQueue->Signal(_fence, ++_fenceVal);

	if (_fence->GetCompleteValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	// ID3D12Resource::WriteToSubresource()メソッドのときと同様にテクスチャ情報が転送され、
	// 用意されたテクスチャが貼り付けられて表示されます

	//TITLE: RowPitchは256の倍数でなければならない
	// ID3D12GraphicsCommandList::CopyTextureRegion()メソッドを使用する場合、
	// コピー元（中間バッファー）のRowPitchメンバーが256の倍数でないと、
	// 関数の実行に失敗することがあります。
	// デバッグレイヤーのエラーメッセージでも、「256アライメントの規約に違反している」といったメッセージが出ており、
	// コピー元のRowPitchメンバーを256の倍数にそろえる必要があります。
	// 200×200というサイズの画像の場合、1行の大きさが4バイト×200 = 800バイトとなり、256の倍数ではなくなり、
	// ID3D12GraphicsCommandList::CopyTextureRegion()メソッドの実行は失敗するはずです
	
	// コピー元のRowPitchを256アライメントにするためには以下の計算をすればよいです

	src.PlacedFootprint.Footprint.RowPitch =
		img->rowPitch
		+ D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
		- img->rowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

	// D3D12_TEXTURE_DATA_PITCH_ALIGNMENTは256として定義されています
	// 直値の仕様はできるだけ避けたいので、定数を使用します
	// アライメントをそろえる計算はDirectX12では頻繁に出てくるので関数化する

	// アライメントにそろえたサイズを返す
	// @param size 元のサイズ
	// @param alignment アラインメントサイズ
	// @return アライメントをそろえたサイズ
	size_t AlignmentedSize(size_t size, size_t alignment)
	{
		return size + alignment - size % alignment;
	}

	src.PlacedFootprint.Footprint.RowPitch =
		AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
		* img->height;

	// もちろんimg->slicePitchをアライメントしてもダメです
	// RowPitchがアライメントされているため、「アライメントされたサイズ×高さ」とする必要があり、
	// このようなコードになります

	// このままでは、まだずれが生じてしまい、正常に表示されません
	// 原因は元データをコピーする際に、元データのRowPitchが800なのにもかかわらず、
	// バッファーのRowPitchが1024となっているためです
	// そこで1行ごとにコピーして行頭が合うようにします

	auto srcAddress = img->pixels;

	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNEMENT);

	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(srcAddress,
			rowPitch,
			mapforImg);	// コピー

		// 1行ごとのつじつまを合わせる
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}

	// これで期待した画像を表示できたはずです・
	// ID3D12Device::GetCopyableFootprints()メソッドを使うと、あるべきピッチを取得することも可能です。
	
	//TITLE: d3dx12.hの利用例
	// ヒープ設定とリソース設定を細かく設定して頂点バッファーを作る
		
	D3D12_HEAP_PROPERTIES heapprop = {};	// 頂点バッファー用ヒープ設定
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Width = sizeof(vertices);

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	// 書き直すとこう
	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),	// UPLOADヒープとして
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),	// サイズに応じて適切な設定をしてくれる
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	// CD3DX12～というのが、d3dx12.hが提供しているヘルパー構造体と呼ばれるもので、これを使うことでコードの量がかなり減らせる
	// バリアの設定についても適用
	// バックバッファーの状態をPRESENTからRENDER_TARGETへ遷移させるバリア

	D3D12_RESOURCE_BARRIER BarrierDesc = {};

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	// CD3DX12ヘルパー構造体を使う
	_cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx], D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET)	// これだけで済む
	);

	// 他のd3dx12.hの利用例
	// ID3D12GraphicsCommandList::CopyTextureRegion()メソッドを利用した転送→UpdateSubresource()
#endif
#pragma endregion

#pragma region CHAPTER6
// 行列による座標変換
#if CHAPTER  == 6

	void Note::main()
	{

		// 行列の基本
		//TITLE: 座標変換操作の順番
		// 座標変換、基本的に行列の乗算を用いて計算する
		// 行列の乗算なので、計算の順序が重要
		//１．回転して平行移動する
		// （平行移動）（回転）
		//２．平行移動してから回転する
		// （回転）（平行移動）

		// 回転は「原点中心」
		// 座標変換における回転は必ず「原点中心」です
		
		//TITLE:行優先と列優先
		// DirectXプログラミングで行列を扱う際には行優先と列優先に気を付けておかなればなりません。
		// ベクトルを行列で表す場合、ベクトルは行列が縦か横だけに並んでいるものを表すことができ、
		// 1 × n もしくは n × 1 の状態になっている「行列の一形態」といえる
		// 行優先/列優先では、そのベクトルの向きが重要になる
		// 列優先では、ベクトルを
		// (x)
		// (y)
		// (z)
		// と表します

		// 行優先ではベクトルを横に表記する（1×n）形式だといえる
		// (x y z)
		// と表す

		// ベクトル（座標）に対して行列で変換をかける場合の乗算の向きが変わってくる
		
		// 列優先の場合
		// (a b c) (x)   (x')
		// (d e f) (y) = (y')
		// (g h i) (z)   (z')
		// のように、左に乗算すべき行列が来ることで変換後の座標が得られる
		// 行列同士も左へ左へと乗算していく。
		
		// 列優先の場合
		//          (a b c)
		// (x y z)  (d e f) =  (x' y' z')
		//          (g h i)
		// となるため、乗算すべき行列を右から掛けることになる
		// 行列同士の演算も右へ右へと乗算していく

		// この違いは重要
		// 特にDirectXでは、C++でXMMATRIX構造体を扱っているときは列優先となり、
		// HLSLで乗算する際には行優先となる

		//TITLE: XMMATRIX構造体
		// DirectX12において、行列はXMMATRIX構造体（DirextXMathライブラリ）を使用して表現する
		// XMMATRIX構造体の演算は行優先であるため、右方向に乗算していく

		// 基本は4x4
		// なお、DirectX（DirectXMath）ではSIMD演算を行うため、行列の乗算といえ4x4行列（XMMATRIX）同士の乗算
		// もしくは4x4行列（XMMATRIX）と４要素ベクトル（XMVECTOR）の乗算となる

		// y軸を中心とした回転を行うにはXMMatrixRotationY()関数を使用し、
		// 平行移動を行うにはXMMatrixTranslation()を使用する
		// もし、回転してから平行移動をする場合は、

		XMMatrixRotationY(XM_1DIV2PI) * XMMatrixTranslation(1, 2, 3);

		// 平行移動してから回転する

		XMMatrixTranslation(1, 2, 3)* XMMatrixRotationY(XM_1DIV2PI);

		// HLSLは行優先
		// シェーダー（HLSL）では行優先のため順序が逆です

		//TITLE: 定数バッファーとシェーダーからの利用
		// CPUで設定した行列は、GPU側にどのように転送されるか
		// 行列は、これまで出てきたような頂点でもテクスチャでもない
		// 「定数データ」として「定数バッファー」を介してGPU側に転送される
		// 定数バッファーの利用手順
		//１．定数バッファーを作る
		//２．定数バッファーの中身をマップで書き換える
		//３．定数バッファービューをディスクリプタヒープの追加する
		//４．ルートシグネチャに定数バッファー参照用のレンジ設定を追加する
		//５．シェーダーから利用する

		//TITLE: 定数バッファーの作成
		// バッファー自体の作り方は頂点バッファーのときと同じ
		// マップで中身を書き換える必要があるので、D3D12_HEAP_TYPE_UPLOADとして設定する
		// バッファー（ID3D12Resourceオブジェクト）の作成はID3D12Device::CreateCommittedResource()メソッドで行う

		// 定数バッファー作成
		ID3D12Resource* constBuff = nullptr;
		ID3D12Device* _dev;
		_dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(256/*必要なバイト数を256アライメントしたバイト数*/),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuff)
		);

		// DirectX12では「定数バッファーのアライメントは256バイトでなければならない」
		// なぜか
		// ＧＰＵを効率よく使用するための仕様
		
		// 単位行列を転送する
		XMMATRIX matrix = XMMatrixIdentity();

		// 定数バッファー作成
		ID3D12Resource* constBuff = nullptr;

		_dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer((sizeof(matrix) + 0xff) & ~0xff),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuff)
		);

		// 256アライメントの計算
		// 第３引数の計算が「256の倍数にする」という演算です
		
		size + (256 - size % 256);

		// size = 50の場合

		50 + (256 - 50 % 256);

		50 + (256 - 50);

		50 + 206;
		
		256;

		// size = 260の場合

		260 + (256 - 260 % 256);

		260 + (256 - 4);

		260 + 252;

		512;

		// 第３引数の式の場合

		(size + 0xff) & ~0xff;

		// 0xff = 256;
		// つまり末尾8ビットがすべて1になっている数
		// ~はビット反転を表す演算子（ビットNOT演算子）
		// ~0xffは、末尾８ビットがすべて０で他はすべて１の値となります
		// 二進法だと
		// 1111 1111 1111 1111 1111 1111 1111 0000 0000
		// この値と&（AND）演算を行っているため、末尾８ビットがすべて０になり256の倍数になる

		//TITLE:マップによる定数のコピー
		// D3D12_HEAP_TYPE_UPLOADでバッファーを作っているのでマップできます
		// ひとまずアンマップの必要はないので、中に行列を書き込む

		XMMATRIX* mapMatrix;	// マップ先を示すポインター
		result = constBuff->Map(0, nullptr, (void**)&mapMatrix);	// マップ
		*mapMatrix = matrix;	// 行列の内容をコピー

		// 代入演算子が使える
		// 「コピーなのでmemcpyを使う」ことなく代入演算子を使うこともできる

		//TITLE: 定数バッファービュー（CBV）
		// テクスチャをシェーダーから見えるようにするためには
		// ・ディスクリプタヒープを作り、
		// ・その中にビュー定義し、
		// ・スロットの割り当てもルートシグネチャで記述する
		// としますが、定数バッファーでも同様に、記述します
		// ディスクリプタヒープ自体は新しく作らなくても、テクスチャ用に作ったものを共用できる
		// ルートパラメーターも、ディスクリプタレンジをもう１つ追加するだけでよい

		//TITLE: ディスクリプタヒープを書き換える
		// テクスチャのために作ったディスクリプタヒープを書き換える
		ID3D12DescriptorHeap* texDescHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

		// シェーダ―から見えるように
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		// マスクは０
		descHeapDesc.NodeMask = 0;

		// ビューは今のところ１つだけ
		descHeapDesc.NumDescriptors = 1;

		// ディスクリプタヒープの種別
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		// 生成
		result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

		// 変数名をかえる
		// 今のままではテクスチャ用に特化しているように見えるので「基本的な情報の受け渡し」としてbasicDescHeapという変数名にする
		// NumDescriptorメンバーを、テクスチャビュー(SRV)と定数バッファービューの２つ必要、ということで１から２にする

		ID3D12DescriptorHeap* basicDescHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

		// シェーダ―から見えるように
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		// マスクは０
		descHeapDesc.NodeMask = 0;

		// SRV1つとCBV1つ
		descHeapDesc.NumDescriptors = 2;

		// ディスクリプタヒープ種別
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		// 生成
		result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

		// NumDescriptorsメンバーの変更により、ディスクリプタヒープ内の「ビュー置き場」の数が２つになりました。
		// 次は、シェーダーリソースビューのあとに定数バッファービューを追加する

		//TITLE: 定数バッファービューの生成
		// シェーダ―リソースビュー生成部分は、
		// ID3D12Device::CreateConstantBufferView()メソッドを呼び出して、定数バッファービューを生成します
		
		_dev->CreateShaderResourceView(texbuff,	// ビューと関連付けるバッファー
			&srvDesc,	// 先ほど設定したテクスチャ設定情報
			basicDescHeap->GetCPUDescriptorHandleForHeapStart()	// ヒープ内の配置位置
			);

		// ID3D12Device::CreateConstantBufferView()
		void CreateConstantBufferView(
			const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc,	// 定数バッファービュー設定
			D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor		// ビュー配置すべき場所の情報（ハンドル）
		);

		// 第１引数（定数バッファービューの設定）は、グラフィックスメモリ上のアドレスとそのサイズを渡せばよい
		// バッファーのアドレスは、すでに定数バッファー用に作ったバッファー(constBuff)のGetGPUVirtualAddress()メソッド
		// を使用して取得。バッファーのサイズもGetDesc().Widthで得られる
		// 第２引数（ビューを配置すべき場所の情報）はD3D12_CPU_DESCRIPTOR_HANDLEという構造体
		// この構造体はptrというメンバー持っており、それが配置すべきアドレスを指している

		// ディスクリプタヒープ上にシェーダーリソースビューと定数バッファービューを設定する場合、
		// 連続したメモリとして配置する
		// ディスクリプタヒープ内のビューとしてはシェーダーリソースビューも定数バッファービューも同じ大きさであり、
		// かつ決められた大きさとなる

		// シェーダ―リソースビューを設定するときに用いたID3D12DescriptorHeap::GetCPUDescriptorHandleForHeapStart()メソッドは、
		// ビューを配置すべき先頭アドレスが入ったハンドルを返す
		// そのうえで定数バッファービューの配置アドレスを知るために、ビュー情報一つあたりの大きさがわかればオフセットできる
		// この大きさ（インクリメントサイズ）を知るための関数がID3D12Device::GetDescriptor::GetDescriptorHandleIncrementSize()メソッド
		
		// ビュー情報１つあたりの大きさは、
		// ・シェーダーリソースビュー（SRV）
		// ・定数バッファービュー（CBV）
		// ・アンノーダードアクセスビュー（UAV）
		// の３つは同じとなっている

		// レンダーターゲットビューおよび深度ステンシルビューの大きさはそれらとは異なるので、注意

		// ID3D12Device::GetDescriptorHandleIncrementSize()メソッドの引数に定義済みの定数を渡すことで
		// インクリメントサイズが得られる
		// 今回はCBVなので、引数にはD3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAVを使用する

		// ID3D12DescriptroHeap::GetCPUDescriptorHandleForHeapStart()メソッドが返すハンドルを取得

		auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

		// 取得したbasicHeapHandleは、ディスクリプタヒープの先頭、つまりシェーダーリソースビューの位置を示す
		// そこで、このハンドルのptrメンバーを大きさ分だけインクリメントする

		basicHeaphandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// basicHeapHandleが定数バッファービューのあるべき場所を示すようになりました
		// シェーダ―リソースビュー作成と定数バッファービュー作成をまとめて書く

		// ディスクリプタの先頭ハンドルを取得して置く
		auto basicHeapHandle = basicDescriptor->GetCPUDescriptorHandleForHeapStart();

		// シェーダ―リソースビューの作成
		_dev->CreateShaderResourceView(
			texbuff,			// ビューと関連付けるバッファー
			&srvDesc,			// 先ほど設定したテクスチャ設定情報
			basicHeapHandle		// 先頭の場所を示すハンドル
		);

		// 次の場所に移動
		basicHeapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

		cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constBuff->GetDesc().Width;

		// 定数バッファービューの作成
		_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

		//TITLE: ルートシグネチャの変更
		// ディスクリプタレンジの設定を変更する
		
		//変更前
		{
			D3D12_DESCRIPTOR_RANGE descTblRange = {};

			descTblRange.NumDescriptors = 1;	// テクスチャ１つ
			descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// 種別はテクスチャ
			descTblRange.BaseShaderRegister = 0;	// 0番スロットから
			descTblRange.OffsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

		// 必要なレンジ数が増えたため、配列にして定数レジスター用の設定を書く
		{
			D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};	// テクスチャと定数の２つ

			// テクスチャ用レジスター０番
			descTblRange[0].NumDescriptors = 1;	// テクスチャ１つ
			descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// 種別はテクスチャ
			descTblRange[0].BaseShaderRegister = 0;	// 0番スロットから
			descTblRange[0].OffsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// 定数用レジスター０番
			descTblRange[1].NumDescriptors = 1;	// 定数１つ
			descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;	// 種別は定数
			descTblRange[1].BaseShaderRegister = 0;	// 0番スロットから
			descTblRange[1].OffsetInDescriptorFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// レジスター番号が重複していても大丈夫か？
			// レジスター番号が重複しても問題ありません。
			// レジスターの種別が違う場合、レジスターは別の場所にあり、それぞれの場所で明確に分けられています。

		}

		// ルートパラメータ
		{
			D3D12_ROOT_PARAMETER rootparam = {};

			rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

			// ディスクリプタレンジのアドレス
			rootparam.DescriptorTable.pDescriptorRanges = descTblRange;

			// ディスクリプタレンジ数
			rootparam.DescriptorTable.NumDescriptorRanges = 1;

			// ピクセルシェーダーから見える
			rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}

		// 最後に設定しているShaderVisibilityメンバーを見ると、「ピクセルシェーダーから見える」
		// 設定になっている
		// 今回作成する定数（行列）は「頂点の座標変換」に用いるものなので、ピクセルシェーダーではなく、頂点シェーダーから見えるようにしないといけない
		// ルートパラメータを分けて設定する
		// D3D12_SHADER_VISIBLITY_ALLという指定もある

		D3D12_ROOT_PARAMETER rootparam[2] = {};

		rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
		rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
		rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
		rootparam[1].DescriptorTable.NumDescriptorRanges = 1;
		rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// ルートパラメータが２つになったので、rootSignatureDesc.NumParametersも2にする

		rootSignatureDesc.NumParameters = 2;	// ルートパラメーター数

		//TITLE: 頂点シェーダーからの定数バッファー（行列）の参照
		// 頂点シェーダーから定数バッファーを参照して、それを頂点シェーダーからの座標変換に使用する

		// 定数の参照を記述する
		Texture2D<float4> tex : register(t0);	// 0番スロットに設定されたテクスチャ
		SamplerState smp : register(s0);		// 0番スロットに設定されたサンプラー

		// 定数バッファー
		cbuffer cbuff0 : register(b0)
		{
			matrix mat;	// 変換行列
		};

		// cbufferとは定数バッファーをまとめるためのキーワード
		// 基本的に、CPU側からの情報はまとめてバイト列として特定のスロットに割り当てられ、
		// シェーダ―側で利用される
		// グローバル定数が複数であっても１つのバッファーとしてまとめて定義する
		// 一見すると記法は構造体のようだが、実際にはただ単に、１つのレジスターと対応関係を持たせるためにまとめているだけ
		// 対応するレジスターを示すのがregister指定子
		// CPU側における「スロット番号」は、GPU側では「レジスター番号」に対応
		// register指定子でスロット番号とレジスター番号を対応させ、その中身をcbufferで定義している
		// 定数レジスターなので、定数レジスター(b)の0番と行列定数を対応させる
		// なお、Shader Model5.1以降では、
		struct Matrix
		{
			matrix mat;	// 変換行列
		};

		ConstantBuffer<Matrix> m : register(b0);

		// のように、C++のテンプレートに似た記法を使い、構造体をまとめてConstantBufferとして定義することも可能

		// 定数バッファーのレジスターは「b」
		// c0ではない
		// 「コンスタント（定数）」だからcと思いがちだが、b
		// cは、バッファーオフセットとして使用される

		// 定数を利用する
		//頂点シェーダーでmatを参照すれば、行列を使うことができる
		// 早速頂点シェーダー内部でmatを使用する
		// 頂点座標に対してmatを乗算することで、頂点座標を行列で変換できる
		output.svpos = mul(mat, pos);

		// シェーダ―での行列演算は列優先であるため、左方向にかける
		// 座標に対して左に行列を置く形で乗算する
		// *演算子ではなくmul()関数を使う
		

		//TITLE: ルートパラメーターとディスクリプタヒープのバインド指定
		//ルートパラメーターとディスクリプタヒープを指定する
		// シェーダ―リソースビュー用の指定（ルートパラメーター番号0番、ディスクリプタヒープは先頭）は以下の通り
		_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		// ルートパラメーター番号１に対して、ディスクリプタヒープの次の場所を対応付け（バインド）する
		// 対応付けの方法は、ビューと作ったときと同様、ハンドルのprtメンバーをID3D12Device::GetDescriptorHandleIncrementSize()メソッドで
		// オフセットさせる

		auto heapHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
		heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		_cmdList->SetGraphicsRootDescriptorTable(1, heapHandle);

		//TITLE: D3D12_SHADER_VISIBILITY_ALL指定
		// この指定を使ってルートパラメーターを全シェーダ―から参照可能にすると、実装がシンプルになる
		// シェーダ―リソースと定数バッファーを同一ルートパラメーターとして扱うことで、
		// ルートパラメーターとディスクリプタヒープのバインドが１回だけで済むようになる
		
		// シェーダ―リソースビューと定数バッファービューが連続しており、レンジも連続しているため
		// ルートパラメーター１つに対して「レンジが２つ」という指定を行えば、いっぺんに２つのレジスター設定ができ、切替コストも軽減できる
		// ルートパラメーターを１本化して、記述する
		{
			D3D12_ROOT_PARAMETER rootparam = {};
			rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

			// 配列先頭アドレス
			rootparam.DescriptorTable.pDescriptorRanges = descTblRange;

			// ディスクリプタレンジ数
			rootparam.DescriptorTable.NumDescriptorRanges = 2;

			// すべてのシェーダーから見える
			rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		// こうすると、次のようにID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable()メソッドの呼び出しが１回で済む

		_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		//TITLE: 行列を変更する(2D編)
		// 2D座標系のピクセルを指定して、画面上に画像を表示する行列を作る
		
		//TITLE: 2D座標の変換行列
		// 画面、つまりクライアント（ビューポート）のサイズがいくつあろうとも、-1～1の範囲で表される
		// 最初に特定のサイズを指定したにもかかわらず、左上は(-1,1)、右下は(1,-1)となってしまい、ピクセル単位の指定が難しくなる

		// 無理矢理にでもピクセル指定をする方法を考える
		// 画面のサイズ1280×720であると仮定する

		//１．(0,0)から(-1,1)への変換
		//２．(1280,720)から(1,-1)への変換
		//３．(1280/2,720/2)から(0,0)への変換

		// 連立１次方程式
		// 












#endif
#pragma endregion








#pragma region CHAPTERX
// テンプレート
#if CHAPTER  == -1


#endif
#pragma endregion

}

#endif // NOTE_MODE

