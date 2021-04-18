#include "Application.h"

#ifdef _DEBUG
int main() {
#else
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	auto& app = Application::Instance();

	if (!app.Init())
	{
		return -1;
	}

	return 0;
}


//// D3Dデバイスが保持しているオブジェクト情報を出力
//void ReportD3DObject()
//{
//	if (_dev == nullptr)
//		return;
//	ID3D12DebugDevice* debugDevice = nullptr;
//	auto result = _dev->QueryInterface(&debugDevice);
//	if (SUCCEEDED(result))
//	{
//		debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
//		debugDevice->Release();
//	}
//}
//
//void releaseResource()
//{
//	// スマートポインタにより不要に
//	{/*
//		safeRelease(_dxgiFactory);
//		safeRelease(_cmdAllocator);
//		safeRelease(_cmdList);
//		safeRelease(_cmdQueue);
//		safeRelease(_swapchain);
//	*/}
//}
//
//#ifdef _DEBUG
//int main() {
//
//#else
//#include<Windows.h>
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
//#endif
//	DebugOutputFormatString("Show window test.");
//
//	HWND hwnd;
//	WNDCLASSEX windowClass = {};
//	CreateGameWindow(hwnd, windowClass);
//
//#ifdef _DEBUG
//	// デバッグレイヤーをオンに
//	EnableDebugLayer();
//#endif
//
//	HRESULT result = InitializeDXGIDevice();
//
//	result = InitializeCommand();
//
//	result = CreateSwapChain(hwnd, _dxgiFactory);
//
//	std::vector<ComPtr<ID3D12Resource>> _backBuffers;
//	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
//
//	result = CreateFinalRenderTarget(rtvHeaps, _backBuffers);
//
//	// フェンスの作成
//	ComPtr<ID3D12Fence> _fence = nullptr;
//	UINT64 _fenceVal = 0;
//	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
//
//	// ウィンドウ表示
//	ShowWindow(hwnd, SW_SHOW);
//
//	auto whiteTex	= CreateWhiteTexture();
//	auto blackTex	= CreateBlackTexture();
//	auto gradTex	= CreateGrayGradationTexture();
//

//	//string strModelPath = "Model/hibiki/hibiki.pmd";
//	//string strModelPath = "Model/satori/satori.pmd";
//	//string strModelPath = "Model/reimu/reimu.pmd";
//	//string strModelPath = "Model/巡音ルカ.pmd";
//	string strModelPath = "Model/初音ミク.pmd";
//	//string strModelPath = "Model/初音ミクVer2.pmd";
//	//string strModelPath = "Model/初音ミクmetal.pmd";
//	//string strModelPath = "Model/咲音メイコ.pmd";
//	//string strModelPath = "Model/ダミーボーン.pmd";//NG
//	//string strModelPath = "Model/鏡音リン.pmd";
//	//string strModelPath = "Model/鏡音リン_act2.pmd";
//	//string strModelPath = "Model/カイト.pmd";
//	//string strModelPath = "Model/MEIKO.pmd";
//	//string strModelPath = "Model/亞北ネル.pmd";
//	//string strModelPath = "Model/弱音ハク.pmd";
//

//	
//

//
//	MSG msg				= {};
//	unsigned int frame	= 0;
//	float angle			= 0.0f;
//	float delta			= 0.005f;
//	float sumDelta		= 0.0f;
//	while (true)
//	{
//		worldMat			= XMMatrixRotationY(angle);
//		viewMat				= XMMatrixLookAtLH(
//			XMLoadFloat3(&eye),
//			XMLoadFloat3(&target),
//			XMLoadFloat3(&up)
//		);;
//		mapMatrix->world	= worldMat;
//		mapMatrix->view		= viewMat;
//		mapMatrix->proj		= projMat;
//		mapMatrix->eye		= eye;
//
//		//angle		+= delta*10.0f;
//		//sumDelta	+= delta;
//		//eye.x		+= delta*20.0f;
//		//target.x	+= delta*20.0f;
//		//if (fabsf(sumDelta) >= 1.0f)
//		//{
//		//	delta *= -1.0f;
//		//}
//
//		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//		if (msg.message == WM_QUIT) {
//			break;
//		}
//
//		// DirectX処理
//		// バックバッファのインデックスを取得
//		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
//
//		D3D12_RESOURCE_BARRIER BarrierDesc = {};
//		BarrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//		BarrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
//		BarrierDesc.Transition.pResource	= _backBuffers[bbIdx].Get();
//		BarrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
//		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
//
//		_cmdList->SetPipelineState(_pipelinestate.Get());
//
//		// レンダーターゲットを指定
//		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
//		rtvH.ptr += static_cast<unsigned long long>(bbIdx) * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//		auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
//
//		_cmdList->ResourceBarrier(1, &BarrierDesc);
//		_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
//
//		// 画面クリア
//		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
//		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
//		_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//
//		_cmdList->RSSetViewports(1, &viewport);
//		_cmdList->RSSetScissorRects(1, &scissorrect);
//
//		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//		_cmdList->IASetVertexBuffers(0, 1, &vbView);
//		_cmdList->IASetIndexBuffer(&ibView);
//
//		_cmdList->SetGraphicsRootSignature(rootsignature.Get());
//
//		// WVP変換行列
//		_cmdList->SetDescriptorHeaps(1, basicDescHeap.GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());
//
//		// マテリアル
//		_cmdList->SetDescriptorHeaps(1, materialDescHeap.GetAddressOf());
//
//		auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();	// ヒープ先頭
//		unsigned int idxOffset = 0;
//
//		auto cbvsrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)* 5;
//		for (auto& m : materials) {
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialH);
//			_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
//			// ヒープポインタとインデックスを次に進める
//			materialH.ptr += cbvsrvIncSize;
//			idxOffset += m.indicesNum;
//		}
//
//		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
//		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
//
//		_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//		//命令のクローズ
//		_cmdList->Close();
//
//		// コマンドリストの実行
//		ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
//		_cmdQueue->ExecuteCommandLists(1, cmdlists);
//		// 待ち
//		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
//
//		while (_fence->GetCompletedValue() != _fenceVal) {
//			;
//		}
//
//		// フリップ
//		_swapchain->Present(1, 0);
//		_cmdAllocator->Reset();							//キューをクリア
//		_cmdList->Reset(_cmdAllocator.Get(), nullptr);	//再びコマンドリストをためる準備
//
//	}
//	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
//
//#ifdef _DEBUG
//	ReportD3DObject();
//#endif
//
//	// 解放
//	//safeRelease(gradTex);
//	//safeRelease(blackTex);
//	//safeRelease(whiteTex);
//	//for (auto toon : toonResources)
//	//{
//	//	safeRelease(toon);
//	//}
//	//for (auto spa : spaResources)
//	//{
//	//	safeRelease(spa);
//	//}
//	//for (auto sph : sphResources)
//	//{
//	//	safeRelease(sph);
//	//}
//	//for (auto tex : textureResources)
//	//{
//	//	safeRelease(tex);
//	//}
//	//safeRelease(materialDescHeap);
//	//safeRelease(materialBuff);
//	//safeRelease(basicDescHeap);
//	//safeRelease(constBuff);
//	//safeRelease(_pipelinestate);
//	//safeRelease(rootsignature);
//	//safeRelease(idxBuff);
//	//safeRelease(vertBuff);
//	//safeRelease(_fence);
//	//safeRelease(dsvHeap);
//	//safeRelease(depthBuffer);
//	//for (auto buf : _backBuffers)
//	//{
//	//	safeRelease(buf);
//	//}
//	//safeRelease(rtvHeaps);
//	//releaseResource();
//
//#ifdef _DEBUG
//	ReportD3DObject();
//#endif
//	//safeRelease(_dev);
//	return 0;
//}
