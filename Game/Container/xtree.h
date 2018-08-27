#ifndef _STX_XTREE_H_
#define _STX_XTREE_H_
#pragma once

#include <xmemory>
#include "../include/xutility.h"

STX_BEGIN_
template<typename Key,
    typename Data,
    typename Value = STX pair<const Key, Data>,
    typename Compair = STX less<Key>,
    typename Alloc = std::allocator<value>>
class xtree
{
public:
    xtree::xtree()
    {
    }

    xtree::~xtree()
    {
    }

private:
    
};


STX_END_

#endif//_STX_XTREE_H_