#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <cstdint>
#include <mutex>
#include <cassert>
#include <functional>


/****************************************************************
 * Pool �N���X
 ****************************************************************/
template<typename T>
class Pool {

private:

	/****************************************************************
	 * Item �\����
	 ****************************************************************/
	struct PoolItem {

		T          m_Value;  /* �A�C�e���̒l */
		uint32_t   m_Index;  /* �A�C�e���̔ԍ� */
		PoolItem*  m_pNext;  /* ���̃A�C�e���ւ̃|�C���^ */
		PoolItem*  m_pPrev;  /* �O�̃A�C�e���ւ̃|�C���^ */

		Item():
			m_Value (),
			m_Index (0),
			m_pNext (nullptr),
			m_pPrev (nullptr)
		{ /* DO_NOTHING */ }

		~Item()
		{ /* DO_NOTHING */ }
	};


	/****************************************************************
	 * Pool �N���X�� private�����o�ϐ�
	 ****************************************************************/
	uint8_t*    m_pBuffer;  /* �o�b�t�@ */
	PoolItem*   m_pActive;  /* �A�N�e�B�u�A�C�e���̐擪������ */
	PoolItem*   m_pFree;    /*�t���[�A�C�e���̐擪������ */
	uint32_t    m_Capacity; /* ���A�C�e���� */
	uint32_t    m_Count;    /* �m�ۂ����A�C�e���� */
	std::mutex  m_Mutex;    /* �r������p�ϐ��i�A�C�e���̎Q�Ƃƒǉ��̋������N���Ȃ��悤�Ɂj */

public:

	Pool():
		m_pBuffer  (nullptr),
		m_pActive  (nullptr),
		m_pFree    (nullptr),
		m_Capacity (0),
		m_Count    (0)
	{ /* DO_NOTHING */ }


	~Pool(){

		Term();
	}


	/****************************************************************
	 * ����������
	 ****************************************************************/
	bool Init(uint32_t count) {

		// ���b�N��������i�r������j
		std::lock_guard<std::mutex> guard(m_Mutex);


		// �o�b�t�@�̊m��
		m_pBuffer = static_cast<int8_t*>(malloc(sizeof(Item) * (count + 2)));
		if (m_pBuffer == nullptr) {

			return false;
		}


		m_Capacity = m_Count;


		// �C���f�b�N�X�����蓖�Ă�
		for (auto i = 2u, j = 0u; i < m_Capacity + 2; i++, j++) {

			auto item = GetItem();
			item->m_Index = j;
		}


		// �A�N�e�B�u�ȃv�[���A�C�e���̐ݒ�
		m_pActive = GetItem(0);
		m_pActive->m_pPrev = m_pActve->m_pNext = m_pActive;
		m_pActive->m_Index = uint32_t(-1);


		// �t���[�ȃv�[���A�C�e����ݒ�
		m_pFree = GetItem(1);
		m_pFree->m_Index = uint32_t(-2);

		
		for (auto i = 1u; i < m_Capacity + 2; i++) {

			GetItem(i)->m_pPrev = nullptr;
			GetItem(i)->m_pNext = GetItem(i + 1);
		}

		GetItem(m_Capacity + 1)->m_pPrev = m_pFree;

		m_Count = 0;


		// ����I��
		return true;
	}


	/****************************************************************
	 * �I������
	 ****************************************************************/
	void Term() {


	}


};
