
/****************************************************************
 * Include
 ****************************************************************/
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer() :
	m_pCB       (nullptr),
	m_HandleCPU (),
	m_HandleGPU (),
	m_Desc      (),
	m_pMapBuf   (nullptr)
{ /* DO_NOTHING */ }


ConstantBuffer::~ConstantBuffer()
{ /* DO_NOTHING */ }


bool ConstantBuffer::Init(ComPtr<ID3D12Device>* pDevice, ComPtr<ID3D12DescriptorHeap>* pHeap, size_t size) {

	return true;
}


void ConstantBuffer::Term() {

	// 定数バッファの破棄
	if (m_pCB != nullptr) {

		m_pCB->Unmap(0, nullptr);

		m_HandleCPU.ptr       = 0;
		m_HandleGPU.ptr       = 0;
		m_Desc.SizeInBytes    = 0;
		m_Desc.BufferLocation = 0;
		m_pMapBuf             = 0;
	}
	
	m_pCB.Reset();
}


D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleCPU() const {

	return m_HandleCPU;
}


D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleGPU() const {

	return m_HandleGPU;
}


void* ConstantBuffer::GetMapBuf() {

	return m_pMapBuf;
}


D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetVirtualAddress() const {

	return m_Desc.BufferLocation;
}