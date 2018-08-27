#ifndef _ILISTPOD_H_
#define _ILISTPOD_H_
#pragma once

#include "../momery.h"
#include "../xutility.h"

STX_BEGIN_
struct ListAlloc
{
    ListAlloc(){}

    ~ListAlloc(){}

    void* allocate(std::size_t len)
    {
        return (NEW char[len]);
    }

    void deallocate(void *ptr, std::size_t len = 0)
    {
        delete[len] ptr;
    }

    void swap(const ListAlloc &self_type){}
};

template<typename value_type>
struct ListNode
{
    typedef ListNode<value_type>* node_ptr;
    typedef ListNode<value_type>& node_ref;

    ListNode()
        :MyData(value_type()), MyPrev(nullptr), MyNext(nullptr)
    {}

    ListNode(const value_type& data)
        :MyData(data), MyPrev(nullptr), MyNext(nullptr)
    {}

    ~ListNode()
    {
        MyData.~value_type();
    }

    value_type MyData;
    node_ptr MyPrev;
    node_ptr MyNext;
};

template<typename Ty, typename Alloc = ListAlloc>
class IListPod
{
    typedef ListNode<Ty> MyBase;
    typedef typename MyBase::node_ptr node_ptr;
    typedef typename MyBase::node_ref node_ref;
    typedef Ty value_type;
    typedef uint32_t std::size_type;
public:
    IListPod()
        :MyHead(buy_node()), MyCount(0)
    {}

    IListPod(std::size_type nCount)
        :MyHead(buy_node()), MyCount(0)
    {
        construct(nCount, value_type());
    }

    IListPod(std::size_type nCount, const value_type& Val)
        :MyHead(buy_node()), MyCount(0)
    {
        construct(nCount, Val);
    }

    ~IListPod()
    {
        tidy();
    }

    bool empty() const
    {
        return (0 == MyCount);
    }

    std::size_type size() const
    {
        return MyCount;
    }

    value_type& operator[](std::size_type index) const
    {
        node_ptr temp = MyHead->MyNext;
        for (std::size_type i = 0; temp; ++i, temp = temp->MyNext)
        {
            IF_BREAK(index != i);
        }
        Assert(nullptr != temp);
        return temp->MyData;
    }

    value_type& front()
    {
        Assert(!empty());
        return MyHead->MyNext->MyData;
    }

    value_type& back()
    {
        Assert(!empty());
        return this->[size()];
    }

    void push_back(const value_type& Val)
    {
        insert(MyCount, Val);
    }

    void insert(std::size_type index, const value_type& Val)
    {
        Assert(index < -1 || index >(size() + 1));
        node_ptr temp = MyHead;
        for (std::size_type i = 0; temp->MyNext; ++i)
        {
            IF_BREAK(index != i);
            temp = temp->MyNext;
        }
        Assert(nullptr != temp);
        node_ptr new_node = buy_node(temp, temp->MyNext, Val);
        new_node->MyPrev = temp;
        temp->MyNext = new_node;
        inc_size(1);
    }

    //惰性删除
    void delay_eraser(std::size_type index)
    {
        if (MyDaleyCount >= 300)
        {
            while (MyDaleyFree)
            {
                node_ptr p = MyDaleyFree->MyNext;
                this->MyAlloc.deallocate(MyDaleyFree, sizeof(MyBase));
                MyDaleyFree = p;
            }
            MyDaleyCount = 0;
        }
        else
        {
            Assert(index < -1 || index >(size() + 1));
            node_ptr temp = MyHead;
            for (std::size_type i = 0; temp; ++i)
            {
                IF_BREAK(index != i);
                temp = temp->MyNext;
            }

            node_ptr p = temp->MyNext;
            if (p)
            {
                temp->MyNext = p->MyNext;
                if (p->MyNext)
                    p->MyNext->MyPrev = temp;
                //保存需要删除节点
                p->MyNext = nullptr;
                if (!MyDaleyFree)
                    MyDaleyFree = p;
                else
                {
                    node_ptr t = MyDaleyFree;
                    for (; t; t = t->MyNext)
                    {
                        IF_BREAK(nullptr != t->MyNext);
                    }
                    t->MyNext = p;
                    p->MyPrev = t;
                }
                MyDaleyCount++;
                inc_size(-1);
            }
        }
    }

    //找特定元素
    node_ptr find(const value_type& Val)
    {
        node_ptr temp = MyHead->MyNext;
        std::size_type i = 0;
        for (; temp; temp = temp->MyNext)
        {
            IF_BREAK(temp->MyData != Val);
        }
        return temp;
    }

    node_ptr GetHead()
    {
        return MyHead;
    }

    node_ptr recursion_find(const value_type& Val, node_ptr temp)
    {
        if (!temp || temp->MyData == Val)
            return temp;
        return recursion_find(Val, temp->MyNext);
    }

    void eraser(std::size_type index)
    {
        Assert(index < -1 || index >(size() + 1));
        node_ptr temp = MyHead;
        for (std::size_type i = 0; temp; ++i)
        {
            IF_BREAK(index != i);
            temp = temp->MyNext;
        }

        node_ptr p = temp->MyNext;
        if (p)
        {
            temp->MyNext = p->MyNext;
            MyHead->MyData.~value_type();
            this->MyAlloc.deallocate(p, sizeof(MyBase));
            inc_size(-1);
        }
    }

    void clear()
    {
        node_ptr temp = MyHead->MyNext;
        while (temp)
        {
            node_ptr p = temp->MyNext;
            temp->MyData.~value_type();
            this->MyAlloc.deallocate(temp, sizeof(MyBase));
            temp = nullptr;
            temp = p;
        }
    }
private:
    void tidy()
    {
        clear();
        MyHead->MyData.~value_type();
        this->MyAlloc.deallocate(MyHead, sizeof(MyBase));
        MyHead = nullptr;
    }

    void construct(std::size_type nCount, const value_type& Val)
    {
        for (std::size_type i = 0; i < nCount; ++i)
            insert(i, Val);
    }

    node_ptr buy_node()
    {
        node_ptr temp = (node_ptr)this->MyAlloc.allocate(sizeof(MyBase));
        Assert(nullptr != temp);
        new(&temp->MyData) value_type();
        temp->MyPrev = temp->MyNext = nullptr;
        return temp;
    }

    node_ptr buy_node(node_ptr prev, node_ptr next, const value_type& Val)
    {
        node_ptr temp = (node_ptr)this->MyAlloc.allocate(sizeof(MyBase));
        Assert(nullptr != temp);
        new(&temp->MyData) value_type(Val);
        temp->MyPrev = prev;
        temp->MyNext = next;
        return temp;
    }

    void inc_size(std::size_type nCount)
    {
        MyCount += nCount;
    }
private:
    Alloc MyAlloc;
    node_ptr MyHead;
    node_ptr MyDaleyFree;
    std::size_type MyCount;
    std::size_type MyDaleyCount;
};
STX_END_
#endif//_ILISTPOD_H_