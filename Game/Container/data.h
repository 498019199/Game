#ifndef STX_DATA_H
#define STX_DATA_H
#pragma once

#include "../momery.h"
#include "char_traits.h"

//字符缓存区Ty(char, wchar_t)
template<typename Ty, uint32_t SIZE>
class BuffData
{
public:
    BuffData()
        :m_ptr(nullptr),m_size(0)
    {

    }

    BuffData(Ty* src, uint32_t size)
    {
        init(src, size);
    }

    BuffData(Ty* src)
    {
        init(src, TCharTrains<Ty>::Length(src));
    }

    ~BuffData()
    {
        clear();
    }

    void resize(uint32_t size)
    {
        clear();
        if (size > SIZE)
            m_ptr = NEW Ty[size + 1];
        else
            m_ptr = m_data;
        m_size = size;
    }

    char* c_str()
    {
        return m_ptr;
    }

    bool empty()
    {
        return (nullptr == m_ptr 
            && 0 == m_size);
    }

    uint32_t size()
    {
        return m_size;
    }

    //是否用过堆内存
    bool isbuffer()
    {
        if (m_size > SIZE)
            return true;
        return false;
    }

    void clear()
    {
        if (m_size > SIZE)
            delete[] m_ptr;
        m_size = 0;
    }
private:
    void init(Ty* src, uint32_t size)
    {
        m_size = size;
        if (m_size > SIZE)
            m_ptr = NEW Ty[m_size + 1];
        else
            m_ptr = m_data;
        TCharTrains<Ty>::Copy(m_ptr, src, m_size);
    }

    BuffData& operator=(const BuffData& that);
private:
    Ty m_data[SIZE];
    Ty* m_ptr;
    uint32_t m_size;
};
#endif//STX_DATA_H