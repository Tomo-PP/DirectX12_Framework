/*******************************************************
 * ���� �\����
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // ���_���W�i�ˉe�s��j
	float2 TexCoord : TEXCOORD;     // uv���W
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
    //output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    float2 uv = input.TexCoord;
    output.Color = float4(uv.x, uv.y, 0.0f, 1.0f); // uv���W�m�F�p�i�ԁFu�����A�΁Fv�����j
    //float2 pos = (input.Position.xy + 1.0f) * 0.5f;
    //output.Color = float4(pos, 0.0f, 1.0f);
    
    return output;
}