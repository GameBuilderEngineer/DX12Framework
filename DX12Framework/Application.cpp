#include "Application.h"
#include "Dx12Wrapper.h"
#include "PMDRenderer.h"
#include "PMDActor.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#ifdef _DEBUG
#include<iostream>
#endif

///@brief コンソール画面にフォーマット付き文字列を表示
///@param format フォーマット
///@param 可変長引数
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	_RPTN(_CRT_WARN, format, valist);
	va_end(valist);
#endif
}

// ウィンドウ定数
const unsigned int window_width = 1280;
const unsigned int window_height = 720;

// ウィンドウプロシージャ
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {	// ウィンドウ破棄時呼び出し
		PostQuitMessage(0);		// OSに対して終了メッセージ
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// ゲームウィンドウの生成
void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);

	//ウィンドウクラス生成＆登録
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc		= (WNDPROC)WindowProcedure;	// コールバック関数の指定
	windowClass.lpszClassName	= _T("DX12Framework");		// アプリケーションクラス名
	windowClass.hInstance		= GetModuleHandle(0);		// ハンドルの取得
	RegisterClassEx(&windowClass);							// アプリケーションクラス

	RECT wrc = { 0,0,window_width, window_height };			// ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);		// ウィンドウのサイズを関数を使って補正する

	// ウィンドウオブジェクトの生成
	hwnd = CreateWindow(
		windowClass.lpszClassName,	// クラス名指定
		_T("DX12フレームワーク"),	// タイトルバーの文字
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

// ウィンドウサイズの取得
SIZE Application::GetWindowSize()const {
	SIZE ret;
	ret.cx = window_width;
	ret.cy = window_height;
	return ret;
}

// ループ処理
void Application::Run()
{
	// ウィンドウ表示
	ShowWindow(_hwnd, SW_SHOW);
	float angle			= 0.0f;
	MSG msg				= {};
	unsigned int frame	= 0;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		// 全体の描画準備
		_dx12->BeginDraw();

		// PMD用の描画パイプラインに合わせる
		_dx12->CommandList()->SetPipelineState(_pmdRenderer->GetPiplineState());
		// ルートシグネチャをPMD用に合わせる
		_dx12->CommandList()->SetGraphicsRootSignature(_pmdRenderer->GetRootSignature());

		_dx12->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		_dx12->SetScene();

		_pmdActor->Update();
		_pmdActor->Draw();

		_dx12->EndDraw();

		// フリップ
		_dx12->Swapchain()->Present(1,0);
	}
}

bool Application::Init() {
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	CreateGameWindow(_hwnd,_windowClass);

	DebugOutputFormatString("Show window test.\n");

	std::string strModelPath = "Model/初音ミク.pmd";
	//string strModelPath = "Model/hibiki/hibiki.pmd";
	//string strModelPath = "Model/satori/satori.pmd";
	//string strModelPath = "Model/reimu/reimu.pmd";
	//string strModelPath = "Model/巡音ルカ.pmd";
	//string strModelPath = "Model/初音ミクVer2.pmd";
	//string strModelPath = "Model/初音ミクmetal.pmd";
	//string strModelPath = "Model/咲音メイコ.pmd";
	//string strModelPath = "Model/ダミーボーン.pmd";//NG
	//string strModelPath = "Model/鏡音リン.pmd";
	//string strModelPath = "Model/鏡音リン_act2.pmd";
	//string strModelPath = "Model/カイト.pmd";
	//string strModelPath = "Model/MEIKO.pmd";
	//string strModelPath = "Model/亞北ネル.pmd";
	//string strModelPath = "Model/弱音ハク.pmd";

	// DirectX12ラッパー生成＆初期化
	_dx12.reset(new Dx12Wrapper(_hwnd));
	if (!_dx12->Initialize())
		return false;

	// imguiの初期化
	if (!InitializeImgui())
		return false;

	// PMD用レンダラー&アクターの初期化
	_pmdRenderer.reset(new PMDRenderer(*_dx12));
	_pmdActor.reset(new PMDActor(strModelPath.c_str(), *_pmdRenderer));

	return true;
}

// imguiの初期化
bool Application::InitializeImgui()
{
	if (ImGui::CreateContext() == nullptr)
	{
		assert(0);
		return false;
	}

	bool blnResult = ImGui_ImplWin32_Init(_hwnd);
	if (!blnResult)
	{
		assert(0);
		return false;
	}

	ImGui_ImplDX12_Init(_dx12->Device().Get(),	// DirectX12デバイス
		NUM_FRAMES_IN_FLIGHT,					// 頂点バッファとインデックスバッファの複合構造体によりGPUの処理を待たずに次のコマンドリスト発行ができるようにしている（推定）
		DXGI_FORMAT_R8G8B8A8_UNORM,				// 書き込み先RTVのフォーマット
		_dx12->GetHeapForImgui().Get(),			// imgui用デスクリプタヒープ
		_dx12->GetHeapForImgui()->GetCPUDescriptorHandleForHeapStart(),		// CPUハンドル
		_dx12->GetHeapForImgui()->GetGPUDescriptorHandleForHeapStart());	// GPUハンドル

	return true;
}

// 後処理
void Application::Terminate() {
	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	UnregisterClass(_windowClass.lpszClassName, _windowClass.hInstance);
	DxDebug _dx12->ReportD3DObject();
}

Application& Application::Instance() {
	static Application instance;
	return instance;
}

Application::Application()
	: _windowClass	{}
	, _hwnd			(nullptr)
	, _dx12			(nullptr)
	, _pmdRenderer	(nullptr)
	, _pmdActor		(nullptr)
{

}

Application::~Application() {

}

