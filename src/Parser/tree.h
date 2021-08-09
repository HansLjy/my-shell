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

class Node {
public:
	virtual void AppendChild(Node*);
	virtual NodeIterator GetIterator();

	virtual Sentence GetSentence();
	virtual void SetSentence(const Sentence& sentence);

	virtual int GetSize();
	virtual Node* GetNode(int id);

	virtual void Print(int step);

	friend class NodeIterator;
};

class CompositeNode : public Node {
public:
	virtual void AppendChild(Node* child);
	virtual NodeIterator GetIterator();
	virtual void PrintOperator() = 0;
	virtual void Print(int step);
	~CompositeNode();

protected:
	virtual int GetSize();
	virtual Node* GetNode(int id);
	std::vector<Node*> _children;
};

class NullNode : public Node {
public:
	virtual void Print(int step);
};

class LeafNode : public Node {
public:
	virtual Sentence GetSentence();
	virtual void SetSentence(const Sentence& sentence);
	virtual void Print(int step);

private:
	Sentence _sentence;
};

class PipedNode : public CompositeNode {
public:
	virtual void PrintOperator();
};

class ParaNode : public CompositeNode {
public:
	virtual void PrintOperator();
};

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

class NodeFactory {
public:
	static NodeFactory* Instance();
	Node* CreateLeafNode();			// 生成叶子节点
	Node* CreateCompositeNode(NodeType type);	// 生成非叶子节点
	Node* CreateNullNode();

private:
	static NodeFactory* theFactory;
};


#endif //MYSHELL_TREE_H
