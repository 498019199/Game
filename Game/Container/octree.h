//八叉树
#ifndef _OCTREE_H_
#define _OCTREE_H_
#pragma once 

#include "../momery.h"
STX_BEGIN_

//基本类型
class OctreeNode
{
protected:
    typedef OctreeNode* node_ptr;
    typedef OctreeNode& node_ref;
    typedef const OctreeNode* const_node_ptr;
    typedef const OctreeNode& const_node_ref;
    typedef std::size_t std::size_type;

    enum node_type
    {
        AGGREGATE_NODE, //分支节点
        BRANCH_NODE,    //总节点
        LEAF_NODE,      //叶子节点
    };

    OctreeNode()
        :m_type(LEAF_NODE)
    {}

    ~OctreeNode(){}

public:
    node_type type() const
    {
        return m_type;
    }

private:
    node_type m_type;
};

//总节点
template<typename T,std::size_t SIZE>
class OctreeAggregate :public OctreeNode
{
private:
    T m_children[2][2][2];
};

//分支节点
class OctreeBranch :public OctreeNode
{
private:
    node_ptr m_children[2][2][2];
};

//叶子节点
template<typename T>
class OctreeLeaf :public OctreeNode
{
private:
    T m_value;
};

template<typename ValTy, std::size_t SIZE = 2, typename Alloc = allocator>
class Octree :public OctreeNode
{
    typedef Alloc alloc_type;
    typedef ValTy value_type;
    typedef Octree<value_type, SIZE> MyTy;
    friend class OctreeAggregate<value_type, SIZE>;
    friend class OctreeBranch;
    friend class OctreeBranch < value_type >;
public:
    Octree(std::size_type nCount, const value_type& value = ValTy())
    {}

    Octree(const MyTy& o)
    {}

    ~Octree()
    {}

    MyTy& operator=(const MyTy& o)
    {
        MyTy tmp(o);
        swap(tmp);
        return *this;
    }

    void swap(const MyTy& o)
    {
        STX swap(this->m_alloc, o.m_alloc);
        STX swap(this->m_root,  o.m_root);
        STX swap(this->m_value, o.m_value);
        STX swap(this->m_size,  o.m_size);
    }
private:
    alloc_type      m_alloc;
    node_ptr        m_root;
    value_type      m_value;
    std::size_type       m_size;
};
STX_END_
#endif //_OCTREE_H_