#include "BasicType.hlsli"
Texture2D<float4> tex:register(t0);		// 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(�x�[�X)
Texture2D<float4> sph:register(t1);		// 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(��Z)
Texture2D<float4> spa:register(t2);		// 1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)

SamplerState smp:register(s0);			// 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��

// �萔�o�b�t�@0
cbuffer SceneData : register(b0) {
	matrix world;		// ���[���h�ϊ��s��
	matrix view;		// �r���[�s��
	matrix proj;		// �v���W�F�N�V�����s��
	float3 eye;			// ���_���W
};
// �萔�o�b�t�@1
cbuffer Material : register(b1) {
	float4 diffuse;		// �f�B�t���[�Y�F
	float4 specular;	// �X�y�L����
	float3 ambient;		// �A���r�G���g
}

float4 BasicPS(BasicType input) : SV_TARGET{
	float3 light		= normalize(float3(1,-1,1));	// ���̌������x�N�g��(���s����)
	float3 lightColor	= float3(0, 0, 1);				// ���C�g�̃J���[(��)

	// �f�B�t���[�Y�v�Z
	float diffuseB = saturate(dot(-light, input.normal));

	// ���̔��˃x�N�g��
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	// �X�t�B�A�}�b�v�pUV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	float4 texColor = tex.Sample(smp, input.uv);

	return max(
		diffuseB 											// �P�x
		* diffuse											// �f�B�t���[�Y�F
		* texColor											// �e�N�X�`���J���[
		* sph.Sample(smp, sphereMapUV)						// �X�t�B�A�}�b�v(��Z)
		+ saturate(spa.Sample(smp, sphereMapUV)) * texColor	// �X�t�B�A�}�b�v(���Z)
		+ float4(specularB * specular.rgb, 1)				// �X�y�L����
		, float4(texColor * ambient, 1)						// �A���r�G���g
	);
}