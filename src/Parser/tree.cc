//
// Created by hansljy on 2021/8/8.
//

#include <unistd.h>
#include <sys/wait.h>

#include "tree.h"
#include "commands.h"
#include "jobpool.h"

void Node::AppendChild(Node *) {}

std::string Node::GetSentence() {return "";}	// 缺省为空
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

std::string CompositeNode::GetSentence() {
	std::string str("(" + _children[0]->GetSentence());
	int size = _children.size();
	for (int i = 1; i < size; i++) {
		str += " " + GetOperator() + " " + _children[i]->GetSentence();
	}
	str += ")";
	return str;
}

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

std::string PipedNode::GetOperator() {
	return "|";
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

std::string ParaNode::GetOperator() {
	return "&";
}

int ParaNode::Execute(bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	bool need_control = (tcgetpgrp(STDIN_FILENO) == getpgrp() && isatty(STDIN_FILENO));
	for (int i = 0; i < size - 1; i++) {
		auto pid = fork();
		if (pid == 0) {
			if (need_control) {
				// 如果需要作业控制
				pid = getpid();			// 子进程的 pid
				setpgid(pid, pid);		// 将子进程加到单独的一个进程组中
				signal (SIGINT, SIG_DFL);
				signal (SIGTSTP, STGHandler);
				signal (SIGQUIT, SIG_DFL);
				signal (SIGTTIN, SIG_DFL);
				signal (SIGTTOU, SIG_DFL);
				signal (SIGCHLD, SIG_DFL);
			}
			// 执行子进程
			_children[i]->Execute(false, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			exit(0);	// 正常运行，返回 0
		} else {
			if (need_control) {
				setpgid(pid, pid);		// 将子进程加到单独的一个进程组中
				auto job_pool = JobPool::Instance();
				job_pool->AddJob(pid, _children[i]->GetSentence());
			}
		}
	}
	_children[size - 1]->Execute(true, infile, outfile, errfile);
	if (!need_control) {
		while (wait(nullptr) > 0); // 等待所有子进程结束
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

std::string LeafNode::GetSentence() {
	std::string str(_sentence[0]);
	int size = _sentence.size();
	for (int i = 1; i < size; i++) {
		// 逐步拼合命令
		str += " " + _sentence[i];
	}
	return str;
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
	bool need_control = (tcgetpgrp(STDIN_FILENO) == getpgrp() && isatty(STDIN_FILENO));
	if (Command::IsExternal(command) && need_control) {
		// 如果是外部命令，并且是前台进程。则需要单独开一个进程来做
		auto pid = fork();
		if (pid == 0) {
			// 在子进程中执行外部命令
			pid = getpid();
			setpgid(pid, pid);
			signal (SIGINT, SIG_DFL);
			signal (SIGTSTP, SIG_DFL);
			signal (SIGQUIT, SIG_DFL);
			signal (SIGTTIN, SIG_DFL);
			signal (SIGTTOU, SIG_DFL);
			signal (SIGCHLD, SIG_DFL);
			tcsetpgrp(STDIN_FILENO, pid);

			command->Execute(_sentence);
			fprintf(stderr, "Invalid command: %s\n", _sentence[0].c_str());
			exit(1);	// 如果能运行到这，中间一定出现了问题
		} else {
			setpgid(pid, pid);
			tcsetpgrp(STDIN_FILENO, pid);
			fprintf(stderr, "Start waiting.");
			waitpid(pid, nullptr, 0);	// 等待子进程结束
			fprintf(stderr, "End waiting.");
			tcsetpgrp(STDIN_FILENO, getpid());
			delete command;
			if (cont) {
				return 0;
			} else {
				exit(0);
			}
		}
	} else {
		// 如果是内部命令，或者是后台进程，只需要在当前的进程里面执行就好了
		int ret = command->Execute(_sentence);
		delete command;
		if (cont) {
			return ret;
		} else {
			exit(0);
		}
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