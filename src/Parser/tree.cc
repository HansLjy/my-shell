//
// Created by hansljy on 2021/8/8.
//

#include "tree.h"

void Node::AppendChild(Node *) {}
NodeIterator Node::GetIterator() {return NullIterator(this);}
Sentence Node::GetSentence() {return Sentence();}	// 缺省为空
void Node::SetSentence(const Sentence &sentence) {}	//	 缺省为空
int Node::GetSize() {return 0;}		// 缺省
Node *Node::GetNode(int id) {return nullptr;}

void Node::Print(int step) {
	for (int i = 0; i < step - 2; i++) {
		fputc(' ', stdout);
	}
	if (step > 0) {
		fputc('+', stdout);
	}
	fputs("--", stdout);
}

/*
 * 复合节点
 */

void CompositeNode::AppendChild(Node* child) {
	_children.push_back(child);
}

NodeIterator CompositeNode::GetIterator() {
	return ForwardIterator(this);	// 直接拷贝没问题
}

void CompositeNode::Print(int step) {
	Node::Print(step);
	PrintOperator();
	for (const auto& child : _children) {
		child->Print(step + 2);
	}
}

int CompositeNode::GetSize() {
	return _children.size();
}

Node *CompositeNode::GetNode(int id) {
	return _children.at(id);
}

CompositeNode::~CompositeNode() {
	for (auto &node : _children) {
		delete node;
	}
}

/*
 * 管道节点
 */

void PipedNode::PrintOperator() {
	fprintf(stdout, "\"|\"\n");
}

/*
 * 并行节点
 */

void ParaNode::PrintOperator() {
	fprintf(stdout, "\"&\"\n");
}

/*
 * 叶子节点
 */

Sentence LeafNode::GetSentence() {
	return _sentence;
}

void LeafNode::SetSentence(const Sentence &sentence) {
	_sentence = sentence;
}

/*
 * 空节点
 */

void NullNode::Print(int step) {
	Node::Print(step);
	fprintf(stdout, "Null Node\n");
}

// 打印叶节点
void LeafNode::Print(int step) {
	Node::Print(step);
	fprintf(stdout, "Command: ");
	for (const auto &word : _sentence) {
		fprintf(stdout, "%s ", word.c_str());
	}
	fputc('\n', stdout);
}

/*
 * 迭代器
 */

NodeIterator::NodeIterator(Node *node) : _node(node) {}
void NodeIterator::First() {}
bool NodeIterator::IsDone() {return true;}
void NodeIterator::Next() {}
Node *NodeIterator::CurNode() {return nullptr;}

ForwardIterator::ForwardIterator(Node *node) : NodeIterator(node), _id(0) {}

void ForwardIterator::First() {
	_id = 0;
}

void ForwardIterator::Next() {
	_id++;
}

bool ForwardIterator::IsDone() {
	return _id >= _node->GetSize();
}

Node *ForwardIterator::CurNode() {
	return _node->GetNode(_id);
}

/*
 * 节点工厂
 */

NodeFactory* NodeFactory::theFactory = nullptr;

NodeFactory *NodeFactory::Instance() {
	if (theFactory == nullptr) {
		theFactory = new NodeFactory;
	}
	return theFactory;
}

Node *NodeFactory::CreateLeafNode() {
	return new LeafNode;
}

NullIterator::NullIterator(Node *node) : NodeIterator(node) {}

Node *NodeFactory::CreateCompositeNode(NodeType type) {
	switch (type) {
		case kPipedNode:
			return new PipedNode;
		case kParaNode:
			return new ParaNode;
		default:
			return nullptr;
	}
}

Node *NodeFactory::CreateNullNode() {
	return new NullNode;
}