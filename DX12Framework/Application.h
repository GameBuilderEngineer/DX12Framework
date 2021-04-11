#pragma once
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