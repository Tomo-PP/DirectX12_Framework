/*******************************************************
 * ���� �\����
 *******************************************************/
struct VSInput{
	
    float3 Position : POSITION;   // �ʒu���W
	float2 TexCoord : TEXCOORD;   // uv���W
	float3 Normal   : NORMAL;     // �@���x�N�g��
	float3 Tangent  : TANGENT;    // �ڐ��x�N�g��
};


/*******************************************************
 * �o�� �\����
 *******************************************************/
struct VSOutput{
	
    float4   Position  : SV_POSITION;  // ���_���W�i�ˉe���W�ɕϊ��ς݁j
	float2   TexCoord  : TEXCOORD;     // uv���W
    float4   WorldPos  : WORLD_POS;    // ���[���h��Ԃł̍��W
    float3   CameraPos : CAMERA_POS;   // �J�������W
    
    float3x3 InvTangentBasis : INV_TANGENT_BASIS;  // �ڐ���Ԃւ̋K��ϊ��s��̋t�s��i�ڐ���Ԃ����[���h��Ԃɕϊ��j
};


/*******************************************************
 * Camera �萔�o�b�t�@�iView�s��j
 *******************************************************/
cbuffer CameraView : register(b0){
    
    float4x4 View : packoffset(c0);  // �J�����s��
}


/*******************************************************
 * Transform �萔�o�b�t�@
 *******************************************************/
cbuffer Transform : register(b1){
	
    float4x4 World      : packoffset(c0);   // ���[���h�s��i�萔�o�b�t�@�̐擪�j
	float4x4 Projection : packoffset(c4);   // �ˉe�s��i�E�B���h�E�Ɏʂ����s����W�A�擪����128�o�C�g��j
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
	
    output.Position  = projectPos;
    output.TexCoord  = input.TexCoord;
    output.WorldPos  = worldPos;
    output.CameraPos = float3(View[0][0], View[0][1], View[0][2]); // View�s��̂P�s�ڂ��J�������W
    
    // ���x�N�g���̌v�Z�i���[���h�s��Ń��[���h��Ԃɕϊ��㐳�K���j
    float3 N = normalize(mul((float3x3)World, input.Normal));
    float3 T = normalize(mul((float3x3)World, input.Tangent));
    // �c��̂P�͊O�ςŋ��߂�
    float3 B = normalize(cross(N, T));
	
    // �ڐ���Ԃւ̕ϊ��s��̋t�s����v�Z
    output.InvTangentBasis = transpose(float3x3(T, B, N));
    
	return output;
}