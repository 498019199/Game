// 2018年7月28日 新增C++11特性
// 创建2016年6月 zhangbei
#ifndef _IARRAYPOD_H_
#define _IARRAYPOD_H_
#pragma once

#include "../Container/momery.h"
#include <vector>
#include <boost/assert.hpp>
struct ArrayAlloc
{
	ArrayAlloc() {}

	~ArrayAlloc() {}

	void* allocate(size_t len)
	{
		return (NEW char[len]);
	}

	void deallocate(void *ptr, size_t len = 0)
	{
		delete[len] ptr;
	}

	void swap(const ArrayAlloc &self_type) {}
};

template<typename ValTy, 
    typename Alloc = ArrayAlloc>
class IArrayPod
{
    typedef IArrayPod <ValTy, Alloc>   Mty;
    typedef uint32_t                    size_type;
    typedef ValTy							value_type;
    typedef ValTy*							node_ptr;
    typedef ValTy&						node_ref;
	typedef ValTy* iterator;
	typedef const ValTy * const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
public:
    IArrayPod()
        :m_head(nullptr),m_capacity(0),m_size(0)
    {}

	IArrayPod(const IArrayPod& rhs)
		:m_head(rhs.begin()), m_capacity(0), m_size(rhs.size())
	{}

	IArrayPod(const value_type& t)
		: m_head(&t), m_capacity(0),m_size(1)
	{}

	IArrayPod(const value_type * data, size_type size)
		: m_head(data), m_capacity(0), m_size(rhs.size())
	{
	}

	IArrayPod(const value_type* begin, const value_type* end)
		: m_head(begin), m_capacity(0), m_size(end - begin)
	{
	}

	template <typename A>
	IArrayPod(std::vector<value_type, A> const & v)
		: m_head(v.data()), m_capacity(0), m_size(v.size())
	{
	}

	// 数组引用
	template <size_type N>
	IArrayPod(const value_type(&arr)[N])
		: m_head(arr), m_capacity(0), m_size(N)
	{
	}

	IArrayPod(const std::initializer_list<value_type>& v)
		: m_head(v.begin() == v.end() ? nullptr : v.begin()), m_capacity(0), m_size(v.size())
	{
	}

    IArrayPod(size_type Count)
        :m_head(nullptr), m_capacity(0), m_size(0)
    {
        expend(Count);
    }

    ~IArrayPod()
    {
        tidy();
    }

    // 返回当前对象个数  
    size_type size() const 
    { 
        return m_size; 
    }

    size_type capacity() const 
    { 
        return m_capacity; 
    }

    size_type max_size() const 
    { 
        return ((size_type)(-1) / sizeof(value_type));
    }

    //查询
    bool empty() const 
    { 
        return (0 == size());
    }
   
    node_ref operator[](size_type off) const
    {
		BOOST_ASSERT_MSG(off > size(), "index uprange");
        return m_head[off];
    }
    
    node_ref at(size_type off) const
    {
		BOOST_ASSERT_MSG(off > size(), "index_uprange");
        return m_head[off];
    }

    // 提供访问函数  
    node_ref front()
    { 
		BOOST_ASSERT(!empty());
        return m_head[0];
    }

    node_ref back()
    { 
		BOOST_ASSERT(!empty());
        return m_head[size() - 1];
    }

    void push_back(const value_type& data)
    {
        size_type index = size();
        insert(index, data);
    }

    void pop_back()
    {
        eraser(size() - 1);
    }

	iterator begin() const
	{
		return m_head;
	}
	iterator end() const
	{
		return m_head + size();
	}

	reverse_iterator rbegin() const
	{
		return reverse_iterator(this->end());
	}

	reverse_iterator rend() const
	{
		return reverse_iterator(this->begin());
	}

	std::vector<value_type> to_vector() const
	{
		return std::vector<value_type>(begin(), end());
	}

    void insert(size_t index, const value_type& data)
    {
        if (size() >= capacity())
        {
            //以1.5倍大小扩展
            resize(0 == size() ? 1
                : (size() + (size() + 1) / 2));
        }
        new (&m_head[index]) value_type(data);
        m_size++;
    }

    //删除
    void eraser(size_t index)
    {
		BOOST_ASSERT(index < size());
        node_ptr p = m_head + index;
/*        node_ptr p2 = p + 1;*/
        memmove(p, (p + 1)
            , ((size() - index - 1) * sizeof(value_type)));
        --m_size;
    }

    void resize(size_type new_size)
    {
        if (this->m_capacity < new_size)
        {
            expend(new_size);
        } 
        else
        {
            while (new_size < this->m_capacity)
            {
                pop_back();
            }
        }
    }

    void clear() 
    {
        for (size_t i = 0; i < m_capacity; ++i)
        {
            m_head[i].~value_type();
        }
        m_capacity = 0;
    }

    void print()
    {
        for (size_t i = 0; i < size(); ++i)
        {
            std::cout << m_head[i] << "\t";
        }
        std::cout << std::endl;
    }
private:
    void tidy()
    {
        clear();
        this->m_alloc.deallocate(m_head, sizeof(value_type) * m_size);
        m_head = nullptr;
        m_size = 0;
    }

    void expend(size_type add_size)
    {
        size_type new_capacity = size() + add_size;
        node_ptr  new_head = (node_ptr)
            m_alloc.allocate(new_capacity * sizeof(value_type));
        memset(new_head, 0, new_capacity * sizeof(value_type));

        if (m_head)
        {
            memmove(new_head, m_head, sizeof(value_type) * m_size);
            this->m_alloc.deallocate(m_head,sizeof(value_type) * m_capacity);
        }

        m_head = new_head;
        m_capacity = new_capacity;
    }
private:
    Alloc m_alloc;
    node_ptr m_head;
    size_type m_size;
    size_type m_capacity;
};

template <typename T>
inline bool operator==(IArrayPod<T> lhs, IArrayPod<T> rhs)
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}
	return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
inline bool operator!=(IArrayPod<T> lhs, IArrayPod<T> rhs)
{
	return !(lhs == rhs);
}

#endif//_IARRAYPOD_H_