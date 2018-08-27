#ifndef _CBLOCK_H_
#define _CBLOCK_H_
#pragma once
#include "public/var_type.h"
//内存池
template<typename Ty, uint32_t SIZE>
class Block
{
	typedef struct element_s
	{
		Ty value;
		element_t* pNext;
	}element_t;

	typedef struct block_s
	{
		element_t[SIZE] blocks;
		block_t *pNext;
	}block_t;
public:
	Block();

	~Block();

	Ty* GetAlloc();
	
	void Free(Ty* pValue);

	void Clear();

	uint32_t GetTotalCount() { return m_nTotal; }

	uint32_t GetActiveCount() { return m_nActive; }

	uint32_t GetFreeCount() { return m_nTotal - m_nActive; }
private:
	block_t *m_blocks;			// 分配内存
	element_t *m_freelst;		// 避免查找，快速分配内存
	uint32_t m_nTotal;
	uint32_t m_nActive;
};

template<typename Ty, uint32_t SIZE>
Block::Block()
{
	m_blocks = nullptr;
	m_freelst = nullptr;
	m_nTotal = 0;
	m_nActive = 0;
}

template<typename Ty, uint32_t SIZE>
Block::~Block()
{
	Clear();
}

template<typename Ty, uint32_t SIZE>
void Block<Ty, SIZE>::Clear()
{
	while (m_blocks)
	{
		block_t* tmp = m_blocks->pNext;
		free(m_blocks);
		m_blocks = tmp;
	}

	m_blocks = m_freelst = nullptr;
	m_nTotal = m_nActive = 0;
}

template<typename Ty, uint32_t SIZE>
void Block<Ty, SIZE>::Free(Ty* pValue)
{
	element_t* ptr = static_cast<element_t*>(
		static_cast<char*>(pValue)-offsetof(element_t, pValue));
	ptr->pNext = m_freelst;
	m_freelst = ptr;	/* 提高cache命中*/
}

template<typename Ty, uint32_t SIZE>
Ty* Block<Ty, SIZE>::GetAlloc()
{
	if (!m_freelst)
	{
		block_t * new_block = malloc(sizeof(Ty) * SIZE);
		new_block->pNext = m_blocks;
		m_blocks = new_block;

		for (int i = 0; i < SIZE; ++i)
		{
			new_block[i].pNext = m_freelst;
			m_freelst = &new_block[i];
		}

		m_nTotal += SIZE;
	}

	element_t* ptr = m_freelst;
	m_freelst = m_freelst->pNext;
	ptr->pNext = nullptr;
	m_nActive++;

	return ptr;
}
#endif//_CBLOCK_H_