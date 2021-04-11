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