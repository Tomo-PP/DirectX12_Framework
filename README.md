## 制作した内容
グラフィックスAPIであるDirectX12とC++を用いて、3Dメッシュモデルの表示を行うライブラリです。
1つの3Dモデルにテクスチャ・法線マップの2つまでリソースを確保しています。

![内容画像１](画像１.png)

## 使用したライブラリ
- DirectXTK\
  テクスチャ用のメモリバッファを生成するライブラリ
- Assimp\
  3Dモデルの頂点情報を読み込むためのライブラリ


## 工夫した点
カメラの情報は一つ生成できればそれ以外は同じ情報を参照するので、
カメラ情報を無駄に生成しないようにコピーを行う処理を作成した。

テクスチャなどのリソースは固定バッファ方式を採用しており、
アクセスする際には一定のオフセットでアクセスできるようになっています。

## 改善点
カメラの回転移動が意図した動きにならない場面があるため、回転の処理を改善したい。

テクスチャが固定オフセット方式であるため、必ず1つのモデルにつきテクスチャ用のメモリが固定で割り当てられる。
そのため、テクスチャが1つしか必要ない場合のモデルに関しても余分にメモリを割り当ててしまったり、
テクスチャを多数所持する3Dモデルに対しては固定の数のみしか反映されないことが起きる。
動的にテクスチャのメモリを割り当てられるように改善したい。

## 参考書籍
- 「Direct3Dゲームグラフィックス実践ガイド」\
- 「DirectX12の魔導書　3Dレンダリングの基礎からMMDモデルを躍らせるまで」\
- 「ゲーム制作者になるための3Dグラフィックス技術」\
