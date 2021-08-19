//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_TREE_H
#define MYSHELL_TREE_H

#include <vector>
#include <string>

typedef std::vector<std::string> Sentence;

enum NodeType {
	kNullType,	// 不应该生成 node
	kPipedNode,	// 用 | 连起来的
	kParaNode,	// 用 & 连起来的
	kSeqNode,	// 用 , 连起来的
	kAndNode,	// 用 && 连起来的
	kOrNode,	// 用 || 连起来的
};

/**
 * 节点的基类
 * @note
 * 	此处我采用了 Composite 设计模式，各种节点同属于 Node 的子类，因此 Node 将要包含足够的
 * 	接口，来保证任何子类的功能都可以通过 Node* 来调用。
 */
class Node {
public:
	/**
	 * 添加子节点
	 * @param child 子节点
	 * @warning 只对复合型节点有用，叶子节点此函数被重载为空函数。
	 */
	virtual void AppendChild(Node* child);

	/**
	 * 获得该语法树子树的命令语句
	 * @return 命令语句
	 */
	virtual std::string GetSentence();

	/**
	 * 设置命令语句
	 * @param sentence
	 * @warning 只对叶子节点有用，复合型节点此函数被重载为空函数。
	 */
	virtual void SetSentence(const Sentence& sentence);	// 设置命令语句

	/**
	 * 打印树状的语法树
	 * @param step 打印起点的缩进，以空格为单位
	 */
	virtual void Print(int step);

	/**
	 * 执行节点对应的命令
	 * @param is_shell 当前函数是否由 shell 本身来调用
	 * @param cont 是否需要返回，如果是 false，命令执行完之后允许直接结束进程
	 * @param infile 标准输入
	 * @param outfile 标准输出
	 * @param errfile 标准错误输出
	 */
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) = 0;	// 执行整棵树对应的指令

	/**
	 * 虚析构函数
	 */
	virtual ~Node() = default;
};

// 复合节点，即 Internal Node
class CompositeNode : public Node {
public:
	virtual std::string GetSentence();		// 取得命令语句
	virtual void AppendChild(Node* child);	// 添加子节点
	// 打印运算符
	virtual std::string GetOperator() = 0;	// 获得运算符
	virtual void Print(int step);			// 打印节点
	~CompositeNode();

protected:
	// 取得节点大小
	// 取得第 id 个节点
	std::vector<Node*> _children;	// 孩子节点
};

// 空节点，用于处理一些边界情况
class NullNode : public Node {
public:
	virtual void Print(int step);	// 打印节点
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);			// 执行
};

// 叶子节点，只包含一条指令
class LeafNode : public Node {
public:
	virtual std::string GetSentence();					// 取得命令
	virtual void SetSentence(const Sentence& sentence);	// 设置命令
	virtual void Print(int step);						// 打印节点
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);

private:
	Sentence _sentence;	// 命令语句
};

// 管道节点，用 | 连接的命令
class PipedNode : public CompositeNode {
public:
	virtual std::string GetOperator();
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);
};

// 并行节点，用 & 连接的命令
class ParaNode : public CompositeNode {
public:
	virtual std::string GetOperator();
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);
};

class SeqNode : public CompositeNode {
public:
	virtual std::string GetOperator();
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);
};

class AndNode : public CompositeNode {
public:
	virtual std::string GetOperator();
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);
};

class OrNode : public CompositeNode {
public:
	virtual std::string GetOperator();
	virtual void Execute(bool is_shell, bool cont, int infile, int outfile, int errfile);
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
