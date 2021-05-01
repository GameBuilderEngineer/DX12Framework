﻿#pragma once
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
#include<memory>

template <class T>
void safeRelease(T* p)
{
	if (p != nullptr)
		p->Release();
	p = nullptr;
}
class Dx12Wrapper;
class PMDRenderer;
class PMDActor;
// シングルトンクラス
class Application
{
private:
	// ここに必要な変数(バッファーやヒープなど)を書く
	// ウィンドウ周り
	WNDCLASSEX _windowClass;
	HWND _hwnd;
	std::shared_ptr<Dx12Wrapper>	_dx12;
	std::shared_ptr<PMDRenderer>	_pmdRenderer;
	std::shared_ptr<PMDActor>		_pmdActor;

	// ゲーム用ウィンドウの生成
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	// D3Dデバイスが保持しているオブジェクト情報を出力
	void ReportD3DObject();

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

	// ループ処理
	void Run();

	// 後処理
	void Terminate();
	SIZE GetWindowSize()const;
	~Application();
};
