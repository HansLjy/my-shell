//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_TREE_H
#define MYSHELL_TREE_H

#include <vector>
#include <string>
#include "commands.h"

typedef std::vector<std::string> Sentence;

enum NodeType {
	kNullType,	// 不应该生成 node
	kPipedNode,	// 用 | 连起来的
	kParaNode,	// 用 & 连起来的
};

class NodeIterator;

// 节点的基类
class Node {
public:
	virtual void AppendChild(Node*);	// 添加子节点
	virtual NodeIterator GetIterator();	// 取得迭代器

	virtual Sentence GetSentence();						// 取得命令语句
	virtual void SetSentence(const Sentence& sentence);	// 设置命令语句

	virtual int GetSize();			// 取得节点大小
	virtual Node* GetNode(int id);	// 取得第 id 个节点

	virtual void Print(int step);	// 打印节点，step 为前面的空格个数

	friend class NodeIterator;
};

// 复合节点，即 Internal Node
class CompositeNode : public Node {
public:
	virtual void AppendChild(Node* child);	// 添加子节点
	virtual NodeIterator GetIterator();		// 取得迭代器
	virtual void PrintOperator() = 0;		// 打印运算符
	virtual void Print(int step);			// 打印节点
	~CompositeNode();

protected:
	virtual int GetSize();			// 取得节点大小
	virtual Node* GetNode(int id);	// 取得第 id 个节点
	std::vector<Node*> _children;	// 孩子节点
};

// 空节点，用于处理一些边界情况
class NullNode : public Node {
public:
	virtual void Print(int step);	// 打印节点
};

// 叶子节点，只包含一条指令
class LeafNode : public Node {
public:
	virtual Sentence GetSentence();						// 取得命令
	virtual void SetSentence(const Sentence& sentence);	// 设置命令
	virtual void Print(int step);						// 打印节点

private:
	Sentence _sentence;	// 命令语句
};

// 管道节点，用 | 连接的命令
class PipedNode : public CompositeNode {
public:
	virtual void PrintOperator();
};

// 并行节点，用 & 连接的命令
class ParaNode : public CompositeNode {
public:
	virtual void PrintOperator();
};

// 子节点迭代器
class NodeIterator {
public:
	NodeIterator(Node* node);
	virtual void First();
	virtual void Next();
	virtual bool IsDone();
	virtual Node* CurNode();

protected:
	Node* _node;
};

class ForwardIterator : public NodeIterator {
public:
	ForwardIterator(Node* node);
	virtual void First();
	virtual void Next();
	virtual bool IsDone();
	virtual Node* CurNode();

private:
	int _id;
};

class NullIterator : public NodeIterator {
public:
	NullIterator (Node* node);
};

// 节点工厂
class NodeFactory {
public:
	static NodeFactory* Instance();	// Singleton 模式
	Node* CreateLeafNode();			// 生成叶子节点
	Node* CreateCompositeNode(NodeType type);	// 生成非叶子节点，type 表示具体的节点类型
	Node* CreateNullNode();			// 生成空节点

private:
	static NodeFactory* theFactory;	// 全局唯一节点工厂
};

#endif //MYSHELL_TREE_H
