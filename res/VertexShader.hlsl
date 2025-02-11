/*******************************************************
 * ���� �\����
 *******************************************************/
struct VSInput{
	
    float3 Position : POSITION;   // �ʒu���W
	float2 TexCoord : TEXCOORD;   // uv���W
};


/*******************************************************
 * �o�� �\����
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // ���_���W�i�ˉe���W�ɕϊ��ς݁j
	float2 TexCoord : TEXCOORD;     // uv���W
};


/*******************************************************
 * Transform �萔�o�b�t�@
 *******************************************************/
cbuffer Transform : register(b0){
	
    float4x4 World      : packoffset(c0);   // ���[���h�s��i�萔�o�b�t�@�̐擪�j
    float4x4 View       : packoffset(c4);   // �r���[�s��i�擪����64�o�C�g��j
	float4x4 Projection : packoffset(c8);   // �ˉe�s��i�E�B���h�E�Ɏʂ����s����W�A�擪����128�o�C�g��j
}


/*******************************************************
 * ���C������
 *******************************************************/
VSOutput main( VSInput input){
	
    VSOutput output = (VSOutput)0;
	
	// ���W�ϊ�
    float4 localPos   = float4(input.Position, 1.0f);   // ���[�J�����W�i���f���������Ă�����W�j���擾�B�i���W�ϊ��̂��߂�4x4�s��ɕϊ��j
    float4 worldPos   = mul(World ,    localPos);       // ���[�J�����W���烏�[���h���W�ɕϊ�
    float4 viewPos    = mul(View,      worldPos);       // ���[���h���W����r���[���W�ɕϊ�
    float4 projectPos = mul(Projection, viewPos);       // �r���[���W����ˉe���W�ɕϊ�
	
    output.Position = projectPos;
    output.TexCoord = input.TexCoord;
	
	return output;
}