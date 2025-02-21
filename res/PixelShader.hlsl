/*******************************************************
 * 入力 構造体
 *******************************************************/
struct VSOutput{
	
    float4   Position  : SV_POSITION;  // 頂点座標（射影座標に変換済み）
	float2   TexCoord  : TEXCOORD;     // uv座標
    float4   WorldPos  : WORLD_POS;    // ワールド空間での座標
    float3   CameraPos : CAMERA_POS;   // カメラ座標
    
    float3x3 InvTangentBasis : INV_TANGENT_BASIS;  // 接線空間への規定変換行列の逆行列（接線空間をワールド空間に変換）
};


/*******************************************************
 * 出力　構造体
 *******************************************************/
struct PSOutput{
	
    float4 Color : SV_TARGET0;  // 出力する色
};


/*******************************************************
 * ライト 定数バッファ（CBV）
 *******************************************************/
cbuffer LightBuffer : register(b1) {
    
    // cが１つでfloat4個分
    float3 LightPosition : packoffset(c0);  /* ライト座標 */
    float3 LightColor    : packoffset(c1);  /* ライトの色 */
};


/*******************************************************
 * マテリアル 定数バッファ（CBV）
 *******************************************************/
cbuffer Material : register(b2) {
    
    // cが１つで16 Byteなのでalphaの値は初めのアライメントのw成分で保存される（Shininessも同様）
    float3 Diffuse   : packoffset(c0);
    float  Alpha     : packoffset(c0.w);
    float3 Specular  : packoffset(c1);
    float  Shininess : packoffset(c1.w);
};


/*******************************************************
 * テクスチャとサンプラー（SRV）
 *******************************************************/
SamplerState ColorSmp  : register(s0);   /* サンプラー */
Texture2D    ColorMap  : register(t0);   /* テクスチャ */
Texture2D    NormalMap : register(t1);   /* 法線マップ */



/*******************************************************
 * メイン処理
 *******************************************************/
PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    // uv座標のy成分が逆であったため修正している。もしかするとモデルによって変わるかも
    float2 uv = input.TexCoord;
    //uv.x = 1.0f - uv.x;
    uv.y = 1.0f - uv.y;
    
    
    /* ベクトルの計算 */
    
    // ライトベクトル
    float3 Light = normalize(LightPosition - input.WorldPos.xyz);
    
    // 視線ベクトル
    float3 Eye = normalize(input.CameraPos - input.WorldPos.xyz);
    
    // 法線ベクトル（範囲[0,1]を[-1, 1]に修正・接線空間からワールド空間への変換）
    float3 Normal = NormalMap.Sample(ColorSmp, uv).xyz * 2.0f - 1.0f;
    Normal = mul(input.InvTangentBasis, Normal);
    
    
    // テクスチャをサンプルする
    float4 color = ColorMap.Sample(ColorSmp, uv);

    
    /* 反射の計算 */
    
    // 反射ベクトル
    float3 Reflect = normalize(-reflect(Eye, Normal));

    // 光度計算
    float bright = saturate(dot(Light, Normal));
    // float bright = 1;
    
    output.Color = float4(color.rgb * bright, color.a * Alpha);
    
    return output;
}