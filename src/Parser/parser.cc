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
		theParser->AddOperator(Operator{
			Token ("("), 100, kNullType
		});
		theParser->AddOperator(Operator{
			Token (")"), 100, kNullType
		});
	}
	return theParser;
}

void Parser::AddOperator(const Operator& op) {
	_operators.push_back(op);
}

Operator Parser::GetOperator(int id) {
	if (id < 0 || id >= _operators.size()) {
		throw Exception ("Out of range!");
	}
	return _operators[id];
}

// 把当前栈顶的元素缩成一个元素
void Parser::MergeNode (
	std::stack<int>& operator_stack,
	std::stack<Node*>& operand_stack
) {
	auto node_factory = NodeFactory::Instance();
	int top_id = operator_stack.top();
	Operator top_op = GetOperator(top_id);
	auto node = node_factory->CreateCompositeNode(top_op._type);
	std::stack<Node*> tmp;
	while (!operator_stack.empty() &&  operator_stack.top() == top_id) {
		tmp.push(operand_stack.top());
		operator_stack.pop();
		operand_stack.pop();
	}
	node->AppendChild(operand_stack.top());
	operand_stack.pop();
	while (!tmp.empty()) {
		node->AppendChild(tmp.top());
		tmp.pop();
	}
	operand_stack.push(node);
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
					throw Exception ("Unpaired right bracket!");
				}
				operator_stack.pop();
 			} else if (op_id == kLeftBracket) {
				// 如果是左括号，直接压入栈中
				operator_stack.push(op_id);
			} else {
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
	while (!operator_stack.empty() && operator_stack.top() != kLeftBracket) {
		MergeNode(operator_stack, operand_stack);
	}
	if (!operator_stack.empty()) {	// 如果还有左括号没有匹配完
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