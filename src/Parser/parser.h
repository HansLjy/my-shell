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
 * 	调用 Parse 来将一个字符串转化为语法树
 */
class Parser {
public:
	static Parser* Instance();	// Singleton 模式

	/**
	 * 为解释器添加符号
	 * @param op 待添加符号
	 */
	void AddOperator(const Operator& op);

	/**
	 * 解析
	 * @param str 待解析的命令
	 * @return 语法树的跟节点
	 */
	Node* Parse(char* str);

private:
	static Parser* theParser;

	static const int kLeftBracket = 0;
	static const int kRightBracket = 1;
	static const int kMinValidId = 2;

	/**
	 * 判断符号类型
	 * @param token 待判断的符号
	 * @return
	 * 	>= 0 符号的 ID (特别的： 0 约定为 "("，1 约定为 ")")
	 * 	-1 不是符号
	 */
	int WhichOperator(const Token& token);

	/**
	 * 根据 ID 返回符号
	 * @param id 符号 id
	 * @return 符号
	 */
	Operator GetOperator(int id);

	/**
	 * 使用环境变量进行替换实际的 token
	 * @param token 实际的 token
	 */
	void Replace(std::string& token);

	/**
	 * 把栈顶所有用相同符号链接的操作数压缩成一个
	 * @param operators 符号栈
	 * @param operands 操作数栈
	 */
	void MergeNode (std::stack<int>& operators, std::stack<Node*>& operands);

	/**
	 * 发生 Exception 的时候执行清理工作
	 */
	void ClearUp(std::stack<Node*>&);

	std::vector<Operator> _operators;		// 存储的运算符

};

#endif //MYSHELL_PARSER_H
