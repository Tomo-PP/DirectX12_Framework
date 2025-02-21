/*******************************************************
 * ���� �\����
 *******************************************************/
struct VSOutput{
	
    float4   Position  : SV_POSITION;  // ���_���W�i�ˉe���W�ɕϊ��ς݁j
	float2   TexCoord  : TEXCOORD;     // uv���W
    float4   WorldPos  : WORLD_POS;    // ���[���h��Ԃł̍��W
    float3   CameraPos : CAMERA_POS;   // �J�������W
    
    float3x3 InvTangentBasis : INV_TANGENT_BASIS;  // �ڐ���Ԃւ̋K��ϊ��s��̋t�s��i�ڐ���Ԃ����[���h��Ԃɕϊ��j
};


/*******************************************************
 * �o�́@�\����
 *******************************************************/
struct PSOutput{
	
    float4 Color : SV_TARGET0;  // �o�͂���F
};


/*******************************************************
 * ���C�g �萔�o�b�t�@�iCBV�j
 *******************************************************/
cbuffer LightBuffer : register(b1) {
    
    // c���P��float4��
    float3 LightPosition : packoffset(c0);  /* ���C�g���W */
    float3 LightColor    : packoffset(c1);  /* ���C�g�̐F */
};


/*******************************************************
 * �}�e���A�� �萔�o�b�t�@�iCBV�j
 *******************************************************/
cbuffer Material : register(b2) {
    
    // c���P��16 Byte�Ȃ̂�alpha�̒l�͏��߂̃A���C�����g��w�����ŕۑ������iShininess�����l�j
    float3 Diffuse   : packoffset(c0);
    float  Alpha     : packoffset(c0.w);
    float3 Specular  : packoffset(c1);
    float  Shininess : packoffset(c1.w);
};


/*******************************************************
 * �e�N�X�`���ƃT���v���[�iSRV�j
 *******************************************************/
SamplerState ColorSmp  : register(s0);   /* �T���v���[ */
Texture2D    ColorMap  : register(t0);   /* �e�N�X�`�� */
Texture2D    NormalMap : register(t1);   /* �@���}�b�v */



/*******************************************************
 * ���C������
 *******************************************************/
PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    // uv���W��y�������t�ł��������ߏC�����Ă���B����������ƃ��f���ɂ���ĕς�邩��
    float2 uv = input.TexCoord;
    //uv.x = 1.0f - uv.x;
    uv.y = 1.0f - uv.y;
    
    
    /* �x�N�g���̌v�Z */
    
    // ���C�g�x�N�g��
    float3 Light = normalize(LightPosition - input.WorldPos.xyz);
    
    // �����x�N�g��
    float3 Eye = normalize(input.CameraPos - input.WorldPos.xyz);
    
    // �@���x�N�g���i�͈�[0,1]��[-1, 1]�ɏC���E�ڐ���Ԃ��烏�[���h��Ԃւ̕ϊ��j
    float3 Normal = NormalMap.Sample(ColorSmp, uv).xyz * 2.0f - 1.0f;
    Normal = mul(input.InvTangentBasis, Normal);
    
    
    // �e�N�X�`�����T���v������
    float4 color = ColorMap.Sample(ColorSmp, uv);

    
    /* ���˂̌v�Z */
    
    // ���˃x�N�g��
    float3 Reflect = normalize(-reflect(Eye, Normal));

    // ���x�v�Z
    float bright = saturate(dot(Light, Normal));
    // float bright = 1;
    
    output.Color = float4(color.rgb * bright, color.a * Alpha);
    
    return output;
}