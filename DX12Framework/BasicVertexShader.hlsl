#include"BasicType.hlsli"
Texture2D<float4> tex:register(t0);	// 0�ŃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp:register(s0);		//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��

//�萔�o�b�t�@
cbuffer cbuff0 : register(b0) {
	matrix world;	// ���[���h�ϊ��s��
	matrix viewproj;// �r���[�v���W�F�N�V�����s��
};

//���_�V�F�[�_�[
BasicType BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL, 
	float2 uv:TEXCOORD, 
	min16uint2 boneno : BONE_NO, 
	min16uint weight : WEIGHT) 
{
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	output.svpos = mul(mul(viewproj, world), pos);	// �V�F�[�_�ł͗�D��Ȃ̂Œ���
	normal.w = 0;	// ���s�ړ������𖳌��ɂ���
	output.normal = mul(world, normal);// �@���ɂ����[���h�ϊ����s��
	output.uv = uv;
	return output;
}