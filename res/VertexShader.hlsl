/*******************************************************
 * 入力 構造体
 *******************************************************/
struct VSInput{
	
    float3 Position : POSITION;   // 位置座標
	float2 TexCoord : TEXCOORD;   // uv座標
	float3 Normal   : NORMAL;     // 法線ベクトル
	float3 Tangent  : TANGENT;    // 接線ベクトル
};


/*******************************************************
 * 出力 構造体
 *******************************************************/
struct VSOutput{
	
    float4   Position  : SV_POSITION;  // 頂点座標（射影座標に変換済み）
	float2   TexCoord  : TEXCOORD;     // uv座標
    float4   WorldPos  : WORLD_POS;    // ワールド空間での座標
    float3   CameraPos : CAMERA_POS;   // カメラ座標
    
    float3x3 InvTangentBasis : INV_TANGENT_BASIS;  // 接線空間への規定変換行列の逆行列（接線空間をワールド空間に変換）
};


/*******************************************************
 * Camera 定数バッファ（View行列）
 *******************************************************/
cbuffer CameraView : register(b0){
    
    float4x4 View : packoffset(c0);  // カメラ行列
}


/*******************************************************
 * Transform 定数バッファ
 *******************************************************/
cbuffer Transform : register(b1){
	
    float4x4 World      : packoffset(c0);   // ワールド行列（定数バッファの先頭）
	float4x4 Projection : packoffset(c4);   // 射影行列（ウィンドウに写される行列座標、先頭から128バイト先）
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
	
    output.Position  = projectPos;
    output.TexCoord  = input.TexCoord;
    output.WorldPos  = worldPos;
    output.CameraPos = float3(View[0][0], View[0][1], View[0][2]); // View行列の１行目がカメラ座標
    
    // 基底ベクトルの計算（ワールド行列でワールド空間に変換後正規化）
    float3 N = normalize(mul((float3x3)World, input.Normal));
    float3 T = normalize(mul((float3x3)World, input.Tangent));
    // 残りの１つは外積で求める
    float3 B = normalize(cross(N, T));
	
    // 接線空間への変換行列の逆行列を計算
    output.InvTangentBasis = transpose(float3x3(T, B, N));
    
	return output;
}