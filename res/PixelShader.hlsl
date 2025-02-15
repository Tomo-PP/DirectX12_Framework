/*******************************************************
 * ���� �\����
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // ���_���W�i�ˉe�s��j
	float2 TexCoord : TEXCOORD;     // uv���W
    float3 Normal   : NORMAL;       // �@���x�N�g��
    float3 Tangent  : TANGENT;      // �ڐ�x�N�g��
};


/*******************************************************
 * �o�́@�\����
 *******************************************************/
struct PSOutput{
	
    float4 Color : SV_TARGET0;  // �o�͂���F
};


/*******************************************************
 * �e�N�X�`���ƃT���v���[
 *******************************************************/
SamplerState ColorSmp : register(s0);
Texture2D    ColorMap : register(t0);


PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    // �e�N�X�`�����T���v������
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    //output.Color = float4(input.TexCoord, 0.0f, 1.0f);
    
    return output;
}