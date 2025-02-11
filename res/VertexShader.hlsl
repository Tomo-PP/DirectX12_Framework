/*******************************************************
 * 入力 構造体
 *******************************************************/
struct VSInput{
	
    float3 Position : POSITION;   // 位置座標
	float2 TexCoord : TEXCOORD;   // uv座標
};


/*******************************************************
 * 出力 構造体
 *******************************************************/
struct VSOutput{
	
    float4 Position : SV_POSITION;  // 頂点座標（射影座標に変換済み）
	float2 TexCoord : TEXCOORD;     // uv座標
};


/*******************************************************
 * Transform 定数バッファ
 *******************************************************/
cbuffer Transform : register(b0){
	
    float4x4 World      : packoffset(c0);   // ワールド行列（定数バッファの先頭）
    float4x4 View       : packoffset(c4);   // ビュー行列（先頭から64バイト先）
	float4x4 Projection : packoffset(c8);   // 射影行列（ウィンドウに写される行列座標、先頭から128バイト先）
}


/*******************************************************
 * メイン処理
 *******************************************************/
VSOutput main( VSInput input){
	
    VSOutput output = (VSOutput)0;
	
	// 座標変換
    float4 localPos   = float4(input.Position, 1.0f);   // ローカル座標（モデルが持っている座標）を取得。（座標変換のために4x4行列に変換）
    float4 worldPos   = mul(World ,    localPos);       // ローカル座標からワールド座標に変換
    float4 viewPos    = mul(View,      worldPos);       // ワールド座標からビュー座標に変換
    float4 projectPos = mul(Projection, viewPos);       // ビュー座標から射影座標に変換
	
    output.Position = projectPos;
    output.TexCoord = input.TexCoord;
	
	return output;
}