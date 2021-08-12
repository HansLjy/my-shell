//
// Created by hansljy on 2021/8/8.
//

#include <unistd.h>
#include <sys/wait.h>

#include "tree.h"
#include "commands.h"

void Node::AppendChild(Node *) {}

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

// 重定向还要重新搞
int PipedNode::Execute(bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	bool is_background = (tcgetpgrp(STDIN_FILENO) != getpgrp());	// 当前是不是前台进程
	auto ppid = getpid();	// 父进程的 pid
	for (int i = 0; i < size - 1; i++) {
		auto pid = fork();
		if (pid == 0) {
			pid = getpid();	// 子进程的 pid
			if (is_background) {
				// 如果父进程是后台进程组，将子进程加到父进程的进程组中
				setpgid(pid, ppid);
			} else {
				// 如果父进程是前台进程，将子进程加到新的进程组中
				setpgid(pid, pid);
			}
			// 执行子进程
			_children[i]->Execute(false, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			exit(0);	// 正常运行，返回 0
		} else {
			if (is_background) {
				// 如果父进程是后台进程组，将子进程加到父进程的进程组中
				setpgid(pid, ppid);
			} else {
				// 如果父进程是前台进程，将子进程加到新的进程组中
				setpgid(pid, pid);
			}
		}
	}
	auto pid = fork();
	// 发起前台进程
	if (pid == 0) {
		if (is_background) {
			// 如果父进程是后台进程组，将子进程加到父进程的进程组中
			setpgid(pid, ppid);
		} else {
			// 如果父进程是前台进程，将子进程加到新的进程组中
			setpgid(pid, pid);
		}
		_children[size - 1]->Execute(false, infile, outfile, errfile);
		exit(0);	// 完成之后退出
	} else {
		if (is_background) {
			// 如果父进程是后台进程组，将子进程加到父进程的进程组中
			setpgid(pid, ppid);
			waitpid(pid, nullptr, 0);
		} else {
			// 如果父进程是前台进程，将子进程加到新的进程组中
			setpgid(pid, pid);
			tcsetpgrp(STDIN_FILENO, pid);			// 把控制权转让给子进程
			waitpid(pid, nullptr, 0);	// 等待子进程完成
			tcsetpgrp(STDIN_FILENO, getpid());		// 把控制转回到当前进程
		}
	}
	if (cont) {
		return 0;
	} else {
		exit(0);
	}
}

/*
 * 并行节点
 */

void ParaNode::PrintOperator() {
	fprintf(stdout, "\"&\"\n");
}

int ParaNode::Execute(bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	bool is_background = (tcgetpgrp(STDIN_FILENO) != getpgrp());	// 当前是不是前台进程
	auto ppid = getpid();	// 父进程的 pid
	for (int i = 0; i < size - 1; i++) {
		auto pid = fork();
		if (pid == 0) {
			pid = getpid();	// 子进程的 pid
			if (is_background) {
				// 如果父进程是后台进程组，将子进程加到父进程的进程组中
				setpgid(pid, ppid);
			} else {
				// 如果父进程是前台进程，将子进程加到新的进程组中
				setpgid(pid, pid);
			}
			// 执行子进程
			_children[i]->Execute(false, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			exit(0);	// 正常运行，返回 0
		} else {
			if (is_background) {
				// 如果父进程是后台进程组，将子进程加到父进程的进程组中
				setpgid(pid, ppid);
			} else {
				// 如果父进程是前台进程，将子进程加到新的进程组中
				setpgid(pid, pid);
			}
		}
	}
	auto pid = fork();
	// 发起前台进程
	if (pid == 0) {
		if (is_background) {
			// 如果父进程是后台进程组，将子进程加到父进程的进程组中
			setpgid(pid, ppid);
		} else {
			// 如果父进程是前台进程，将子进程加到新的进程组中
			setpgid(pid, pid);
		}
		exit(0);	// 完成之后退出
	} else {
		if (is_background) {
			// 如果父进程是后台进程组，将子进程加到父进程的进程组中
			setpgid(pid, ppid);
			waitpid(pid, nullptr, 0);
		} else {
			// 如果父进程是前台进程，将子进程加到新的进程组中
			setpgid(pid, pid);
			tcsetpgrp(STDIN_FILENO, pid);			// 把控制权转让给子进程
			waitpid(pid, nullptr, 0);	// 等待子进程完成
			tcsetpgrp(STDIN_FILENO, getpid());		// 把控制转回到当前进程
		}
	}
	if (cont) {
		return 0;
	} else {
		exit(0);
	}
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

int LeafNode::Execute(bool cont, int infile, int outfile, int errfile) {
	CommandFactory* factory = CommandFactory::Instance();
	auto command = factory->GetCommand(_sentence[0]);
	command->Redirect(0, infile);
	command->Redirect(1, outfile);
	command->Redirect(2, errfile);
	int ret = command->Execute(_sentence);
	delete command;
	if (cont) {
		return ret;
	} else {
		exit(0);
	}
}

/*
 * 空节点
 */

void NullNode::Print(int step) {
	Node::Print(step);
	fprintf(stdout, "Null Node\n");
}

// 执行命令，并不做什么
int NullNode::Execute(bool cont, int infile, int outfile, int errfile) {
	return 0;
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