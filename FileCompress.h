```c++
#pragma once
#include<queue>

template<class W>
struct HaffmanTreeNode
{
	HaffmanTreeNode<W>* _left;
	HaffmanTreeNode<W>* _right;
	HaffmanTreeNode<W>* _parent;
	W _w;

	HaffmanTreeNode(const W& w)
		:_w(w)
		, _left(NULL)
		, _right(NULL)
		, _parent(NULL)
	{}
};

template<class W>
class HaffmanTree
{
	typedef HaffmanTreeNode<W> Node;
public:
	HaffmanTree()
		:root(NULL)
	{}
	
	struct NodeCompare
	{
		bool operator()(const Node* l, const Node* r)
		{
			return l->_w > r->_w;
		} 
	};
	HaffmanTree(W* w, size_t n,const W& invalid)
	{
		//构建Huffman tree
		//优先级队列，默认为大堆
		priority_queue < Node*, vector<Node*>, NodeCompare> minheap;
		for (size_t i = 0; i < n; ++i)
		{
			if (w[i] != invalid)
				minheap.push(new Node(w[i]));
		}
		while (minheap.size() > 1)
		{
			Node* left = minheap.top();
			minheap.pop();
			Node* right = minheap.top();
			minheap.pop();
			Node* parent = new Node(left->_w + right->_w);
			parent->_left = left;
			parent->_right = right;

			left->_parent = parent;
			right->_parent = parent;

			minheap.push(parent);
		}
		_root = minheap.top();
	}
	~HaffmanTree()
	{
		Destory(_root);
		_root = NULL;
	}
	void Destory(Node* root)
	{
		if (root)
			return;
		Destory(root->_left);
		Destory(root->_right);
		delete root;
	}
	Node* GetRoot()
	{
		return _root;
	}
private:
	HaffmanTree(const HaffmanTree<W>& t);
	HaffmanTree<W>& operator = (const HaffmanTree<W>& t);
protected:
	Node* _root;
};	
```	
