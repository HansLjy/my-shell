//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_PARSER_H
#define MYSHELL_PARSER_H

#include <string>
#include <vector>
#include <stack>
#include "tree.h"

typedef std::string Token;	// 标记

struct Operator {
	Token _token;
	int _rank;		// 优先级，越小代表优先级越高
	NodeType _type;	// 生成对应的节点类型
};

/*
 * Parser：命令解释器
 * 简介：
 * 	此命令解释器只能处理非常简单的语法，但是对于 shell 来说足够了
 * 	通过 AddOperator 函数来按照优先级从高到底的顺序添加运算符。
 * 	调用 Parse 来将一个字符串转化为语法树
 */
class Parser {
public:
	static Parser* Instance();	// Singleton 模式

	/*
	 * AddOperator(op)
	 * 向 Parser 中添加一个符号 op
	 */
	void AddOperator(const Operator& op);
	Node* Parse(char* str);

private:
	static Parser* theParser;

	static const int kLeftBracket = 0;
	static const int kRightBracket = 1;
	static const int kMinValidId = 2;

	/*
	 * int WhichOperator(token)
	 * 返回符号的 ID
	 * 返回值：
	 * 	>= 0 符号的 ID
	 * 		特别的： 0 约定为 "("，1 约定为 ")"
	 * 	-1 不是符号
	 */
	int WhichOperator(const Token& token);
	Operator GetOperator(int id);			// 根据 ID 返回符号
	void MergeNode (std::stack<int>&, std::stack<Node*>&);
	std::vector<Operator> _operators;		// 存储的运算符

	void ClearUp(std::stack<Node*>&);		// 发生 exception 的时候完成清理工作

};

#endif //MYSHELL_PARSER_H
