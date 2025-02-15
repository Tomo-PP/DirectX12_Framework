#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <cstdint>
#include <mutex>
#include <cassert>
#include <functional>


/****************************************************************
 * Pool クラス
 ****************************************************************/
template<typename T>
class Pool {

private:

	/****************************************************************
	 * Item 構造体
	 ****************************************************************/
	struct PoolItem {

		T          m_Value;  /* アイテムの値 */
		uint32_t   m_Index;  /* アイテムの番号 */
		PoolItem*  m_pNext;  /* 次のアイテムへのポインタ */
		PoolItem*  m_pPrev;  /* 前のアイテムへのポインタ */

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
	 * Pool クラスの privateメンバ変数
	 ****************************************************************/
	uint8_t*    m_pBuffer;  /* バッファ */
	PoolItem*   m_pActive;  /* アクティブアイテムの先頭を示す */
	PoolItem*   m_pFree;    /*フリーアイテムの先頭を示す */
	uint32_t    m_Capacity; /* 総アイテム数 */
	uint32_t    m_Count;    /* 確保したアイテム数 */
	std::mutex  m_Mutex;    /* 排他制御用変数（アイテムの参照と追加の競合が起きないように） */

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
	 * 初期化処理
	 ****************************************************************/
	bool Init(uint32_t count) {

		// ロックをかける（排他制御）
		std::lock_guard<std::mutex> guard(m_Mutex);


		// バッファの確保
		m_pBuffer = static_cast<int8_t*>(malloc(sizeof(Item) * (count + 2)));
		if (m_pBuffer == nullptr) {

			return false;
		}


		m_Capacity = m_Count;


		// インデックスを割り当てる
		for (auto i = 2u, j = 0u; i < m_Capacity + 2; i++, j++) {

			auto item = GetItem();
			item->m_Index = j;
		}


		// アクティブなプールアイテムの設定
		m_pActive = GetItem(0);
		m_pActive->m_pPrev = m_pActve->m_pNext = m_pActive;
		m_pActive->m_Index = uint32_t(-1);


		// フリーなプールアイテムを設定
		m_pFree = GetItem(1);
		m_pFree->m_Index = uint32_t(-2);

		
		for (auto i = 1u; i < m_Capacity + 2; i++) {

			GetItem(i)->m_pPrev = nullptr;
			GetItem(i)->m_pNext = GetItem(i + 1);
		}

		GetItem(m_Capacity + 1)->m_pPrev = m_pFree;

		m_Count = 0;


		// 正常終了
		return true;
	}


	/****************************************************************
	 * 終了処理
	 ****************************************************************/
	void Term() {


	}


};
