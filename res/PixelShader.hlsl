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
    //output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    float2 uv = input.TexCoord;
    output.Color = float4(uv.x, uv.y, 0.0f, 1.0f); // uv座標確認用（赤：u成分、緑：v成分）
    //float2 pos = (input.Position.xy + 1.0f) * 0.5f;
    //output.Color = float4(pos, 0.0f, 1.0f);
    
    return output;
}