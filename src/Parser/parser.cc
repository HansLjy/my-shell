//
// Created by hansljy on 2021/8/8.
//

#include <assert.h>
#include <stack>
#include "parser.h"
#include "deliminator.h"
#include "tree.h"
#include "exceptions.h"

Parser* Parser::theParser = nullptr;

Parser *Parser::Instance() {	// 使用时再初始化
	if (theParser == nullptr) {
		theParser = new Parser;
		// 添加左括号
		theParser->AddOperator(Operator{
			Token ("("), 100, kNullType
		});
		// 添加右括号
		theParser->AddOperator(Operator{
			Token (")"), 100, kNullType
		});
	}
	return theParser;
}

void Parser::AddOperator(const Operator& op) {
	// 添加运算符，直接加入 _operators 数组即可
	_operators.push_back(op);
}

Operator Parser::GetOperator(int id) {
	// 获取 id 对应的运算符，直接从 _operator 中返回即可
	return _operators[id];
}

// 把当前栈顶的元素缩成一个元素
// 警告：调用此函数的时候务必保证符号栈上至少有一个有效的符号
void Parser::MergeNode (
	std::stack<int>& operator_stack,
	std::stack<Node*>& operand_stack
) {
	auto node_factory = NodeFactory::Instance();	// 节点工厂
	int top_id = operator_stack.top();				// 当前栈顶的运算符 id
	Operator top_op = GetOperator(top_id);			// 当前栈顶的运算符
	auto node = node_factory->CreateCompositeNode(top_op._type);
	std::stack<Node*> tmp;	// 用于逆序输出栈上的元素
	while (!operator_stack.empty() &&  operator_stack.top() == top_id) {
		// 把所有用同一符号连接起来的节点合成一个
		tmp.push(operand_stack.top());
		operator_stack.pop();
		operand_stack.pop();
	}
	node->AppendChild(operand_stack.top());	// 节点个数应该比操作符多一个
	operand_stack.pop();
	while (!tmp.empty()) {
		node->AppendChild(tmp.top());
		tmp.pop();
	}
	operand_stack.push(node);
}

// 为了实现 exception safe,需要在异常发生的时候回收垃圾
void Parser::ClearUp(std::stack<Node *>& stack) {
	while (!stack.empty()) {
		delete stack.top();
		stack.pop();
	}
}

Node *Parser::Parse(char *str) {
	NodeFactory* node_factory = NodeFactory::Instance();
	Deliminator deliminator(str);
	std::string token;

	std::stack<Node*> operand_stack;			// 操作数栈
	std::stack<int> operator_stack;				// 操作符栈
	Sentence sentence;							// 累积命令
	while (!(token = deliminator.GetNext()).empty()) {
		int op_id = WhichOperator(token);
		if (op_id == -1) {	// 如果不是运算符
			sentence.push_back(token);
		} else {
			// 新建叶子节点，把刚才读入的命令放入其中
			if (!sentence.empty()) {
				auto node = node_factory->CreateLeafNode();
				node->SetSentence(sentence);
				operand_stack.push(node);
				sentence.clear();
			}
			if (op_id >= kMinValidId) {	// 如果是符号
				Operator cur_op = GetOperator(op_id);
				while (!operator_stack.empty()) {
					int top_id = operator_stack.top();
					Operator top_op = GetOperator(top_id);
					if (top_op._rank >= cur_op._rank) {
						break;	// 如果找到了第一个优先级更低的，退出循环
					}
					MergeNode(operator_stack, operand_stack);
				}
				operator_stack.push(op_id);
			} else if (op_id == kRightBracket) {
				// 如果是右括号，直接把和其匹配的左括号之后的东西全部压缩为一个节点
				while (!operator_stack.empty() && operator_stack.top() != kLeftBracket) {
					MergeNode(operator_stack, operand_stack);
				}
				if (operator_stack.empty()) {
					ClearUp(operand_stack);
					throw Exception ("Unpaired right bracket!");
				}
				operator_stack.pop();
 			} else if (op_id == kLeftBracket) {
				// 如果是左括号，直接压入栈中
				operator_stack.push(op_id);
			} else {
				ClearUp(operand_stack);
				throw Exception ("What the hack is this ID?");
			}
		}
	}
	if (!sentence.empty()) {	// 如果还有命令没有加载进去
		auto node = node_factory->CreateLeafNode();
		node->SetSentence(sentence);
		sentence.clear();
		operand_stack.push(node);
	}
	if (operand_stack.size() == operator_stack.size()) {
		operand_stack.push(node_factory->CreateNullNode());
	}
	if (operand_stack.size() != operator_stack.size() + 1) {
		ClearUp(operand_stack);
		throw Exception("Unexpected token, need operator to connect them");
	}
	while (!operator_stack.empty() && operator_stack.top() != kLeftBracket) {
		MergeNode(operator_stack, operand_stack);
	}
	if (!operator_stack.empty()) {	// 如果还有左括号没有匹配完
		ClearUp(operand_stack);
		throw Exception("Unpaired Left Bracket!");
	}
	Node* ret = operand_stack.top();
	operand_stack.pop();
	assert(operand_stack.empty());
	return ret;
}

int Parser::WhichOperator(const Token& token) {
	int size = _operators.size();
	for (int i = 0; i < size; i++) {
		if (_operators[i]._token == token) {
			return i;
		}
	}
	return -1;
}