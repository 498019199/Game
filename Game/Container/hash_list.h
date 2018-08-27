/*2016/9/9  1.�Խṹ������ȡ�������ȡ��ʱִ�п����������
2.buy_node�ڷ����ⷶΧд�룬�����ڴ汨������ڴ����

2016/9/10 1.��xml���ݵ���hash_list�������ڴ�й¶�ˣ����첻��������
2016/9/14 1.����expend�������⣬��Ԫ������Ԫ�ء�ָ������¡�Ԫ���ڱ��ж�ʧ
2016/9/15 �����ֵ���ñ���

2018/8/23 ����const_iterator,�����������������
:HashIteratorBase(const_cast<node_ptr>(ptr1), const_cast<self_ptr>(ptr2)) ��������
*/
#ifndef _STX_HASH_hash_list_H_
#define _STX_HASH_hash_list_H_
#pragma once

#include <iostream>

#include "momery.h"
#include "tchar.h"

//��ϣ���ڴ����
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

//�ڵ㶨��
template <typename Valty>
struct node
{
	typedef typename Valty::key_type    key_type;
	typedef typename Valty::value_type  value_type;
	typedef typename Valty::hash_t      hash_t;
	typedef typename Valty::size_type   size_type;
	typedef typename Valty::node_type   node_type;
	typedef typename Valty::node_ptr    node_ptr;

	hash_t					hash;      //��ϣֵ
	value_type           data;      //����
	node_ptr             next;      //ָ����һ��
	key_type             name[1];   //������

private:
	node_type& operator=(const node_type&);
};

//��ϣ�������
template <typename Valty>
class HashIteratorBase /*iterator_base/ *<Valty>*��֪��Ϊʲô���� 2017/8/27 /*/
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
			//��Ϊ��ϣ��Ľڵ㲻���������ӵģ�����ɢ���������У�����Ҫ���β���
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

	node_ptr    Mypnode;      //�ڵ�ָ��
	self_ptr    Mypthis;      //thisָ��
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
			return "";//��Ҫ����char*Ϊnullֱ�ӷ���""��
					  //�������C�����ַ�����������
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
			return "";//��Ҫ����char*Ϊnullֱ�ӷ���""��
					  //�������C�����ַ�����������
		}
		return this->Mypnode->name;
	}

	const value_type& get_data() const
	{
		return (this->Mypnode->data);
	}
};
//��ϣ�������б�
//\@ Key    �ַ���
//\@ Data   ��������
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
				//�Ա���ڵ���ʾִ������
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
		//��Ҫ�ڷ�����ڴ���д������
		node_ptr ptrnode = (node_ptr)m_alloc.allocate(count);
		memset(ptrnode, 0, count);
		memcpy(ptrnode->name, key, len);
		//����д��Ҳ��֪����֪�������ڱ�ȡ����ʱ��
		//���һ���ṹ�壬�Խṹ��ִ�п��������С�ֻҪ��ָ����ʾ���þͻ��ڴ汨��
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

	//�����ϣ��������
	void expend()
	{
		//ÿ����չ������СӦ���Ǹ�����
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
	node_ptr    *m_head;         //��ϣ��ͷ�ڵ�
	node_ptr    m_tail;         //��ϣ��ͷ�ڵ�
	size_type   m_size;          //�ڵ����
	size_type   m_capacity;      //������С
};

#endif//_STX_HASH_hash_list_H_
