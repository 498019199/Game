/*2016/9/9  1.对结构保存与取出，解决取出时执行拷贝构造出错
2.buy_node在分配外范围写入，导致内存报错完成内存测试

2016/9/10 1.用xml数据导入hash_list发现又内存泄露了，今天不搞了郁闷
2016/9/14 1.发现expend函数问题，表元素链接元素。指向错误导致。元素在表中丢失
2016/9/15 添加右值引用避免

2018/8/23 增加const_iterator,构造哪里。。。有问题
:HashIteratorBase(const_cast<node_ptr>(ptr1), const_cast<self_ptr>(ptr2)) 先这样了
*/
#ifndef _STX_HASH_hash_list_H_
#define _STX_HASH_hash_list_H_
#pragma once

#include <iostream>

#include "momery.h"
#include "tchar.h"

//哈希表内存分配
struct  HashListAlooc
{
	HashListAlooc(){}

	~HashListAlooc(){}

	void* allocate(size_t len)
	{
		return (NEW char[len]);
	}

	void deallocate(void *ptr, size_t len = 0)
	{
		delete[len] ptr;
		ptr = nullptr;
	}

	void swap(const HashListAlooc &self_type){}
};

//节点定义
template <typename Valty>
struct node
{
	typedef typename Valty::key_type    key_type;
	typedef typename Valty::value_type  value_type;
	typedef typename Valty::hash_t      hash_t;
	typedef typename Valty::size_type   size_type;
	typedef typename Valty::node_type   node_type;
	typedef typename Valty::node_ptr    node_ptr;

	hash_t					hash;      //哈希值
	value_type           data;      //数据
	node_ptr             next;      //指向下一个
	key_type             name[1];   //键名字

private:
	node_type& operator=(const node_type&);
};

//哈希表迭代器
template <typename Valty>
class HashIteratorBase /*iterator_base/ *<Valty>*不知道为什么报错 2017/8/27 /*/
{
public:
	typedef typename Valty::Ch         Ch;
	typedef typename Valty::key_type   key_type;
	typedef typename Valty::value_type value_type;
	typedef typename Valty::node_ptr   node_ptr;
	typedef typename Valty::node_ref   node_ref;
	typedef typename Valty::Mty*       self_ptr;
	typedef typename Valty::size_type  size_type;
	typedef HashIteratorBase<Valty>        Myit;
public:
	HashIteratorBase() = default;

	explicit HashIteratorBase(node_ptr ptr1, self_ptr ptr2)
		:Mypnode(ptr1), Mypthis(ptr2)
	{
	}

	bool operator==(const Myit refnode) const
	{
		//if (Ch::compare(GetKey(), refnode.GetKey())
		//	&& Ch::equip(GetKey(), refnode.GetKey()))
		//	return true;
		if (Mypnode == refnode.Mypnode)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Myit refnode) const
	{
		return !(*this == refnode);
	}

	bool empty() const
	{
		if (nullptr == Mypnode)
			return true;
		return false;
	}
protected:
	node_ptr inc(node_ptr ptrnode)
	{
		node_ptr tmp = ptrnode->next;
		if (nullptr != tmp)
		{
			return tmp;
		}
		else
		{
			//因为哈希表的节点不是依次增加的，而是散落在数组中，所以要依次查找
			tmp = Mypthis->goto_next(ptrnode);
			if (nullptr != tmp)
				return tmp;
		}
		return nullptr;
	}

	node_ptr dec(node_ptr ptrnode)
	{
		return nullptr;
	}

	node_ptr    Mypnode;      //节点指针
	self_ptr    Mypthis;      //this指针
};

template <typename Valty>
class HashIterator :public HashIteratorBase<Valty>
{
	using HashIteratorBase::key_type;
	using HashIteratorBase::value_type;
	using HashIteratorBase::node_ptr;
	using HashIteratorBase::self_ptr;
	typedef HashIterator<Valty>        Myit;
public:
	HashIterator() = default;

	explicit HashIterator(node_ptr ptr1, self_ptr ptr2)
		:HashIteratorBase(ptr1, ptr2)
	{
	}

	Myit& operator++()
	{
		node_ptr tmp = Mypnode->next;
		if (nullptr == tmp)
			tmp = inc(Mypnode);
		Mypnode = tmp;
		return *this;
	}

	Myit& operator++(int)
	{
		iterator *tmp = this;
		++this;
		return tmp;
	}

	node_ptr get_node() const
	{
		return this->Mypnode;
	}

	key_type* get_key() const
	{
		if (nullptr == this->Mypnode)
		{
			return "";//不要返回char*为null直接返回""，
					  //避免调用C库中字符串操作报错
		}
		return this->Mypnode->name;
	}

	value_type& get_data() const
	{
		return (this->Mypnode->data);
	}
};

template <typename Valty>
class ConstHashIterator :public HashIteratorBase<Valty>
{
	using HashIteratorBase::key_type;
	using HashIteratorBase::value_type;
	using HashIteratorBase::node_ptr;
	using HashIteratorBase::self_ptr;
	typedef ConstHashIterator<Valty>        Myit;
public:
	ConstHashIterator() = default;

	 explicit ConstHashIterator(const node_ptr ptr1, const Valty* const ptr2)
		:HashIteratorBase(const_cast<node_ptr>(ptr1), const_cast<self_ptr>(ptr2))
	{
	}

	Myit& operator++()
	{
		node_ptr tmp = Mypnode->next;
		if (nullptr == tmp)
			tmp = inc(Mypnode);
		Mypnode = tmp;
		return *this;
	}

	Myit& operator++(int)
	{
		iterator *tmp = this;
		++this;
		return tmp;
	}

	const node_ptr get_node() const
	{
		return this->Mypnode;
	}

	const key_type* get_key() const
	{
		if (nullptr == this->Mypnode)
		{
			return "";//不要返回char*为null直接返回""，
					  //避免调用C库中字符串操作报错
		}
		return this->Mypnode->name;
	}

	const value_type& get_data() const
	{
		return (this->Mypnode->data);
	}
};
//哈希表：分离列表法
//\@ Key    字符串
//\@ Data   数据类型
template <typename Key,
	typename Value,
	typename Alloc = HashListAlooc>
class hash_list
{
public:
	typedef hash_list <Key, Value, Alloc>   Mty;
	typedef node<Mty>                   MyBase;
	typedef Key                         key_type;
	typedef Trains<Key>           Ch;
	typedef Value                       value_type;
	typedef Alloc                       alloc_type;
	typedef MyBase                      node_type;
	typedef MyBase*                     node_ptr;
	typedef MyBase&                     node_ref;
	typedef uint32_t                    size_type;
	typedef uint32_t                    hash_t;
	typedef HashIterator<Mty>           iterator;
	typedef ConstHashIterator<Mty>           const_iterator;

	hash_list()
		:m_head(nullptr), m_tail(nullptr),m_size(0), m_capacity(0)
	{}

	hash_list(size_type n)
	{
		resize(4);
	}

	~hash_list()
	{
		tidy();
	}

	Mty& operator=(const Mty& that)
	{
		return *this;
	}

	void resize(size_type n)
	{
		if (n > 0)
		{
			m_capacity = n;
			m_head = (node_ptr*)m_alloc.allocate(m_capacity * sizeof(node_ptr));
			memset(m_head, 0, m_capacity * sizeof(node_ptr));
		}
	}

	bool assign(const key_type* key, const value_type& value)
	{
		auto it = find(key);
		if (it.empty())
		{
			insert(key, value);
		}
		else
		{
			it.get_data() = std::move(value);
			//tmp.swap(const_cast<value_type& >(value));
		}

		return true;
	}

	void insert(const key_type* key, const value_type& value)
	{
		if (m_size >= m_capacity)
			expend();

		hash_t nbucket = bucket(Ch::hash_value(key));
		node_ptr tmp = m_head[nbucket];

		node_ptr pnew_node = buy_node(key, value);
		m_head[nbucket] = pnew_node;
		pnew_node->next = tmp;
		++m_size;
	}

	void insert(const key_type* key, value_type&& value)
	{
		if (m_size >= m_capacity)
			expend();

		hash_t nbucket = bucket(Ch::hash_value(key));
		node_ptr tmp = m_head[nbucket];

		node_ptr pnew_node = buy_node(key, value);
		m_head[nbucket] = pnew_node;
		pnew_node->next = tmp;
		++m_size;
	}

	bool eraser(const key_type* key)
	{
		hash_t nhash = Ch::hash_value(key);
		hash_t nbucket = bucket(nhash);
		node_ptr ptrnode = m_head[nbucket];

		for (; ptrnode; ptrnode = ptrnode->next)
			if (nhash == ptrnode->hash
				&& Ch::compare(key, ptrnode->name))
			{
			remove_node(ptrnode, nbucket);
			return true;
			}
		return false;
	}

	iterator find(const key_type* key)
	{
		hash_t nhash = Ch::hash_value(key);
		hash_t nbucket = bucket(nhash);
		if (nbucket >= capacity())
			return end();

		node_ptr ptr = m_head[nbucket];
		for (; ptr; ptr = ptr->next)
			if (nhash == ptr->hash && Ch::compare(key, ptr->name))
				break;
		return iterator(ptr, this);
	}

	const_iterator find(const key_type* key) const
	{
		hash_t nhash = Ch::hash_value(key);
		hash_t nbucket = bucket(nhash);
		if (nbucket >= capacity())
			return end();

		node_ptr ptr = m_head[nbucket];
		for (; ptr; ptr = ptr->next)
			if (nhash == ptr->hash && Ch::compare(key, ptr->name))
				break;
		return const_iterator(ptr, this);
	}

	iterator begin()
	{
		size_type i = 0;
		for (; i < m_capacity; ++i)
			if (nullptr != m_head[i]
				&& 0 != m_head[i]->hash)
				break;
		if (0 == m_capacity)
		{
			return end();
		}
		return iterator(m_head[i], this);
	}

	iterator end() 
	{
		return iterator(nullptr, this);
	}

	const_iterator begin() const 
	{
		size_type i = 0;
		for (; i < m_capacity; ++i)
			if (nullptr != m_head[i]
				&& 0 != m_head[i]->hash)
				break;
		if (0 == m_capacity)
		{
			return end();
		}
		return const_iterator(m_head[i], this);
	}

	const_iterator end() const
	{
		return const_iterator(nullptr, this);
	}

	inline bool empty() const noexcept
	{
		return (0 == m_size);
	}

		inline size_type size() const noexcept
	{
		return m_size;
	}

		inline size_type capacity() const noexcept
	{
		return m_capacity;
	}

		inline size_type max_size() const noexcept
	{
		return m_alloc.max_size();
	}

	void swap(Mty& that) noexcept
	{
		this->m_alloc.swap(that.m_alloc);
		std::swap(this->m_head, that.m_head);
		std::swap(this->m_size, that.m_size);
		std::swap(this->m_capacity, that.m_capacity);
	}

		void clear()
	{
		for (size_type i = 0; i < m_capacity; ++i)
		{
			node_ptr ptrnode = m_head[i];
			while (ptrnode)
			{
				node_ptr tmp = ptrnode->next;
				//对保存节点显示执行析构
				ptrnode->data.~value_type();
				this->m_alloc.deallocate(ptrnode, sizeof(MyBase));
				ptrnode = tmp;
			}
		}
		m_size = m_capacity = 0;
	}

	void print()
	{
		std::cout << "\n hash list element print:" << std::endl;
		for (size_type i = 0; i < m_capacity; ++i)
		{
			node_ptr tmp = m_head[i];
			if (tmp)
				for (; tmp; tmp = tmp->next)
					std::cout << " name = " << tmp->name
					<< "  data = " << tmp->data
					<< "  index = " << i
					<< "  adress = " << static_cast<void*>(tmp)
					<< "\n";
		}
	}

	size_type bucket(const hash_t value) const noexcept
	{
		if (0 == capacity())
			return value;
		return value % m_capacity;
	}

	node_ptr goto_next(node_ptr ptrnode)
	{
		node_ptr tmp;
		size_type star = this->bucket(ptrnode->hash) + 1;
		for (; star < this->capacity(); ++star)
		{
			tmp = this->m_head[star];
			if (nullptr != tmp)
				return tmp;
		}
		return nullptr;
	}
private:
	void tidy()
	{
		clear();
		this->m_alloc.deallocate(m_head, m_size);
	}

	node_ptr buy_node(const key_type *key, const value_type& value)
	{
		size_type len = (Ch::length(key) + 1) * sizeof(key_type);
		size_type count = len + sizeof(MyBase);
		//不要在分配的内存外写入数据
		node_ptr ptrnode = (node_ptr)m_alloc.allocate(count);
		memset(ptrnode, 0, count);
		memcpy(ptrnode->name, key, len);
		//这样写我也不知道，知道保存在表取出来时，
		//如果一个结构体，对结构体执行拷贝构造中《只要用指针显示调用就会内存报错》
		new(&ptrnode->data) value_type(value);
		//ptrnode->data = std::move(value);
		ptrnode->hash = Ch::hash_value(key);
		return ptrnode;
	}

	void remove_node(node_ptr ptrnode, hash_t nbucket)
	{
		node_ptr tmp = m_head[nbucket];
		if (ptrnode == tmp)
		{
			tmp = tmp->next;
			m_head[nbucket] = tmp;
		}
		else
			for (; tmp; tmp->next)
				if (ptrnode == tmp->next)
					tmp = ptrnode->next;
		ptrnode->data.~value_type();
		m_alloc.deallocate(ptrnode);
	}

	//扩大哈希索引数组
	void expend()
	{
		//每次扩展容量大小应该是个质数
		size_type new_capacity = m_capacity * 2 + 1;
		node_ptr  *new_head = (node_ptr*)
			m_alloc.allocate(new_capacity * sizeof(node_ptr));
		memset(new_head, 0, new_capacity * sizeof(node_ptr));

		if (m_head)
		{
			for (size_type i = 0; i < m_capacity; ++i)
			{
				node_ptr ptrnode = m_head[i];
				if (nullptr != ptrnode)
				{
					node_ptr tmp = ptrnode->next;
					while (tmp)
					{
						node_ptr p = tmp->next;
						hash_t nbucket =
							Ch::hash_value(tmp->name) % new_capacity;
						node_ptr s = new_head[nbucket];
						new_head[nbucket] = tmp;
						tmp->next = s;
						tmp = p;
					}
					hash_t nbucket =
						Ch::hash_value(ptrnode->name) % new_capacity;
					node_ptr s = new_head[nbucket];
					new_head[nbucket] = ptrnode;
					ptrnode->next = s;
				}
			}
			m_alloc.deallocate(m_head);
		}
		m_head = new_head;
		m_capacity = new_capacity;
	}
private:
	alloc_type  m_alloc;
	node_ptr    *m_head;         //哈希表头节点
	node_ptr    m_tail;         //哈希表头节点
	size_type   m_size;          //节点个数
	size_type   m_capacity;      //分配表大小
};

#endif//_STX_HASH_hash_list_H_
