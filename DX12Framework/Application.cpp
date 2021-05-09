#include "Application.h"
#include "Dx12Wrapper.h"
#include "PMDRenderer.h"
#include "PMDActor.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include <Effekseer.h>
#include <EffekseerRendererDX12.h>
#include <EffekseerSoundXAudio2.h>

#pragma comment(lib,"Effekseer.lib")
#pragma comment(lib,"EffekseerRendererDX12.lib")
#pragma comment(lib,"EffekseerSoundXAudio2.lib")
#pragma comment(lib,"LLGI.lib")
#ifdef _DEBUG
#include<iostream>
#endif

//------------------------------------------------------------------------------
// Effekseer
//------------------------------------------------------------------------------
// ---基本---
// エフェクトレンダラー
EffekseerRenderer::RendererRef _efkRenderer;
// エフェクトマネージャー
Effekseer::ManagerRef _efkManager;
// ---DX12やVulkan,metalなどのコマンドリスト系への対応用---
// メモリプール
Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> _efkMemoryPool;
// コマンドリスト(DX12/Vulkan対応用)
Effekseer::RefPtr<EffekseerRenderer::CommandList> _efkCmdList;
// ---エフェクト再生用---
// エフェクト本体（エフェクトファイルに対応）
Effekseer::EffectRef _effect;
// エフェクトハンドル（再生中のエフェクトに対応）
Effekseer::Handle _efkHandle;
// エフェクト用サウンド
::EffekseerSound::SoundRef _efkSound;

// XAudio2の初期化
static IXAudio2* g_xa2 = nullptr;
static IXAudio2MasteringVoice* g_xa2_master = nullptr;
IXAudio2* GetIXAudio2() { return g_xa2; }

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
const unsigned int window_width = 1920;
const unsigned int window_height = 1080;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ウィンドウプロシージャ
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {	// ウィンドウ破棄時呼び出し
		PostQuitMessage(0);		// OSに対して終了メッセージ
		return 0;
	}
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
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

		// imgui 描画前処理
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Demo window
		if (_show_demo_window)
			ImGui::ShowDemoWindow(&_show_demo_window);

		// Simple Basic
		{
			ImGui::Begin("Rendering Test Menu");
			ImGui::SetWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);
			ImGui::End();
		}

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &_show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &_show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&_clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// Another Simple Window
		if (_show_another_window)
		{
			ImGui::Begin("Another Window", &_show_another_window);
			ImGui::Text("Helllo from another window!");
			if (ImGui::Button("Close Me"))
				_show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();

		// コマンドリストへ描画情報をセット
		_dx12->CommandList()->SetDescriptorHeaps(1, _dx12->GetHeapForImgui().GetAddressOf());	// imgui用デスクリプタをセット
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _dx12->CommandList().Get());		// コマンドリストへ描画情報をセット

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
	, _hwnd					(nullptr)
	, _dx12					(nullptr)
	, _pmdRenderer			(nullptr)
	, _pmdActor				(nullptr)
	, _show_demo_window		(true)
	, _show_another_window	(false)
{
	_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

Application::~Application() {

}

