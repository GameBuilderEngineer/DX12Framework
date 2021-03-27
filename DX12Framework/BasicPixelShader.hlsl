#include "BasicType.hlsli"
Texture2D<float4> tex:register(t0);		// 0番スロットに設定されたテクスチャ(ベース)
Texture2D<float4> sph:register(t1);		// 1番スロットに設定されたテクスチャ(乗算)
Texture2D<float4> spa:register(t2);		// 1番スロットに設定されたテクスチャ(加算)

SamplerState smp:register(s0);			// 0番スロットに設定されたサンプラ

// 定数バッファ0
cbuffer cbuff0 : register(b0) {
	matrix world;		// ワールド変換行列
	matrix view;		// ビュー行列
	matrix proj;		// プロジェクション行列
};
// 定数バッファ1
cbuffer Material : register(b1) {
	float4 diffuse;		// ディフューズ色
	float4 specular;	// スペキュラ
	float3 ambient;		// アンビエント
}

float4 BasicPS(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light, input.normal);

	// スフィアマップ用UV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	float4 texColor = tex.Sample(smp, input.uv);

	return max(
		float4(brightness, brightness, brightness, 1)		// 輝度
		* diffuse											// ディフューズ色
		* texColor											// テクスチャカラー
		* sph.Sample(smp, sphereMapUV)						// スフィアマップ(乗算)
		+ saturate(spa.Sample(smp, sphereMapUV))*texColor	// スフィアマップ(加算)
		, float4(texColor*ambient, 1)						// アンビエント
		);
}