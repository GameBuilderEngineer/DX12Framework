#pragma once

#include<d3d12.h>
#include<DirectXMath.h>
#include<vector>
#include<wrl.h>

class PMDActor
{
private:
	struct Transform
	{
		// 内部に持っているXMMATRIXメンバが16バイトアライメントであるため
		// Transformをnewする際には16バイト境界に確保する
		void* operator new(size_t size);
		DirectX::XMMATRIX world;
	};

};

