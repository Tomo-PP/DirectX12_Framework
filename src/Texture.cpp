
/****************************************************************
 * Include
 ****************************************************************/
#include "Texture.h"
#include "FileUtil.h"
#include <iostream>


Texture::Texture():
	m_pResource  (nullptr),
	m_HandleCPU  (),
	m_HandleGPU  ()
{ /* DO_NOTHING */ }


Texture::~Texture()
{ /* DO_NOTHING */ }


bool Texture::Init(
	ID3D12Device*                 pDevice, 
	ID3D12DescriptorHeap*         pHeap, 
	const wchar_t*                filename, 
	DirectX::ResourceUploadBatch& batch,
	size_t                        HeapCount)
{

	if (pDevice == nullptr || pHeap == nullptr || filename == nullptr) {

		std::cout << "Texture Error : 引数がnullptr" << std::endl;
		return false;
	}

	// ファイルパスの検索
	std::wstring texturePath;
	if (!SearchFilePath(filename, texturePath)) {

		std::cout << "Texture Error : テクスチャが見つからなかった" << std::endl;
		return false;
	}

	// リソースの生成（この関数で一気にリソースを生成できる。リソース設定からしなくてもよい）
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		pDevice,                             /* デバイスを渡す */
		batch,                               /* batch */
		texturePath.c_str(),                 /* テクスチャまでのパス（ワイド文字をマルチバイトに変換して渡す必要あり）*/
		m_pResource.GetAddressOf(),          /* テクスチャのアドレスを取得 */
		true);                               /*  */
	if (FAILED(hr)) {

		std::cout << "Texture Error : テクスチャリソースの生成エラー" << std::endl;
		return false;
	}

	// インクリメントサイズを取得（シェーダー用（SRV）のメモリのインクリメントサイズ）
	auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// CPUディスクリプタハンドルとGPUディスクリプタハンドルをディスクリプタヒープから取得
	auto handleCPU = pHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleGPU = pHeap->GetGPUDescriptorHandleForHeapStart();

	// テクスチャにディスクリプタハンドルを割り当てる（CBV×4を飛ばす必要あり）
	handleCPU.ptr += incrementSize * HeapCount;
	handleGPU.ptr += incrementSize * HeapCount;

	m_HandleCPU = handleCPU;
	m_HandleGPU = handleGPU;

	// テクスチャの構成を取得（上のライブラリで生成された構成を読み込む）
	auto textureDesc = m_pResource->GetDesc();

	// シェーダリソースビューの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;             /*  */
	SRVDesc.Format                        = textureDesc.Format;                        /*  */
	SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  /*  */
	SRVDesc.Texture2D.MipLevels           = textureDesc.MipLevels;                     /*  */
	SRVDesc.Texture2D.MostDetailedMip     = 0;                                         /*  */
	SRVDesc.Texture2D.PlaneSlice          = 0;                                         /*  */
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;                                      /*  */

	// シェーダーリソースビューの生成
	pDevice->CreateShaderResourceView(
		m_pResource.Get(),          /* リソース */
		&SRVDesc,                   /* SRVの設定 */
		m_HandleCPU);               /* CPUディスクリプタハンドル */

	return true;
}


void Texture::Term() {

	// リソースの破棄
	m_pResource.Reset();
	m_HandleCPU.ptr = 0;
	m_HandleGPU.ptr = 0;

}