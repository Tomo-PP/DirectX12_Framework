/*******************************************************
 * 入力 構造体
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // 頂点座標（射影行列）
	float2 TexCoord : TEXCOORD;     // uv座標
    float3 Normal   : NORMAL;       // 法線ベクトル
    float3 Tangent  : TANGENT;      // 接戦ベクトル
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
    //output.Color = float4(input.TexCoord, 0.0f, 1.0f);
    
    return output;
}