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
	app.Run();
	app.Terminate();

	return 0;
}


//// D3Dデバイスが保持しているオブジェクト情報を出力
//void ReportD3DObject()
//{
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
