#include "BasicType.hlsli"
Texture2D<float4> tex:register(t0);		// 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(�x�[�X)
Texture2D<float4> sph:register(t1);		// 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(��Z)
Texture2D<float4> spa:register(t2);		// 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)

SamplerState smp:register(s0);			// 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��

// �萔�o�b�t�@0
cbuffer cbuff0 : register(b0) {
	matrix world;		// ���[���h�ϊ��s��
	matrix viewproj;	// �r���[�v���W�F�N�V�����s��
};
// �萔�o�b�t�@1
cbuffer Material : register(b1) {
	float4 diffuse;		// �f�B�t���[�Y�F
	float4 specular;	// �X�y�L����
	float3 ambient;		// �A���r�G���g
}

float4 BasicPS(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light, input.normal);

	// �X�t�B�A�}�b�v�pUV
	float2 sphereMapUV = input.normal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	return
		float4(brightness, brightness, brightness, 1)	// �P�x
		* diffuse										// �f�B�t���[�Y�F
		* tex.Sample(smp, input.uv)						// �e�N�X�`���J���[
		* sph.Sample(smp, sphereMapUV)					// �X�t�B�A�}�b�v(��Z)
		+ spa.Sample(smp, sphereMapUV);					// �X�t�B�A�}�b�v(��Z)
}