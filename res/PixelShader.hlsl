/*******************************************************
 * 入力 構造体
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // 頂点座標（射影行列）
	float2 TexCoord : TEXCOORD;     // uv座標
};


/*******************************************************
 * 出力　構造体
 *******************************************************/
struct PSOutput{
	
    float4 Color : SV_TARGET0;  // 出力する色
};


/*******************************************************
 * テクスチャとサンプラー
 *******************************************************/
SamplerState ColorSmp : register(s0);
Texture2D    ColorMap : register(t0);


PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    // テクスチャをサンプルする
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    
    return output;
}