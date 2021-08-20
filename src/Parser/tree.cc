//
// Created by hansljy on 2021/8/8.
//

#include <unistd.h>
#include <sys/wait.h>

#include "tree.h"
#include "commands.h"
#include "jobpool.h"
#include "global.h"

void Node::AppendChild(Node *) {}

std::string Node::GetSentence() {return "";}	// 缺省为空
void Node::SetSentence(const Sentence &sentence) {}	//	 缺省为空
// 缺省

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
	std::string op = GetOperator();
	printf("%s\n", op.c_str());
	for (const auto& child : _children) {
		child->Print(step + 2);
	}
}

CompositeNode::~CompositeNode() {
	for (auto &node : _children) {
		delete node;
	}
}

// 管道节点
void PipedNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	bool is_foreground = tcgetpgrp(STDIN_FILENO) == getpgrp();
	auto jobpool = JobPool::Instance();
	auto pid = fork();
	if (pid == 0) {
		pid = getpid();
		if (is_shell) {
			setpgid(pid, pid);
			tcsetpgrp(STDIN_FILENO, pid);
		}
		signal (SIGINT, SIG_DFL);
		signal (SIGTSTP, SIG_DFL);
		signal (SIGQUIT, SIG_DFL);
		signal (SIGTTIN, SIG_DFL);
		signal (SIGTTOU, SIG_DFL);
		signal (SIGCHLD, SIG_DFL);
		int size = _children.size();
		int pipe_file[2];
		int cur_infile = infile;
		int cur_outfile;
		for (int i = 0; i < size - 1; i++) {
			pipe(pipe_file);
			cur_outfile = pipe_file[1];
			auto pid = fork();
			if (pid == 0) {
				// 执行子进程
				close(pipe_file[0]);
				_children[i]->Execute(false, false, cur_infile, cur_outfile, STDERR_FILENO);
				exit(1);
			}
			if (cur_infile != STDIN_FILENO)
				close(cur_infile);
			if (cur_outfile != STDOUT_FILENO)
				close(cur_outfile);
			cur_infile = pipe_file[0];
		}
		_children[size - 1]->Execute(false, true, cur_infile, outfile, errfile);
		while (waitpid(-1, nullptr, 0) > 0);	// 等待所有子进程结束
		exit(0);
	} else {
		if (is_shell) {
			jobpool->AddJob(pid, GetSentence());
			setpgid(pid, pid);
			tcsetpgrp(STDIN_FILENO, pid);
			int status;
			waitpid(pid, &status, WUNTRACED);
			ReportStatus(pid, status);
			tcsetpgrp(STDIN_FILENO, getpgrp());
		} else {
			waitpid(pid, nullptr, WUNTRACED);
		}
	}
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

std::string PipedNode::GetOperator() {
	return "|";
}

// 并行节点
void ParaNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	bool is_foreground = tcgetpgrp(STDIN_FILENO) == getpgrp();
	auto job_pool = JobPool::Instance();
	int size = _children.size();
	for (int i = 0; i < size - 1; i++) {
		auto pid = fork();
		if (pid == 0) {
			if (is_foreground) {
				// 如果是前台进程
				pid = getpid();			// 子进程的 pid
				setpgid(pid, pid);		// 将子进程加到单独的一个进程组中
				signal (SIGINT, SIG_DFL);
				signal (SIGTSTP, SIG_DFL);
				signal (SIGQUIT, SIG_DFL);
				signal (SIGTTIN, SIG_DFL);
				signal (SIGTTOU, SIG_DFL);
				signal (SIGCHLD, SIG_DFL);
				// 执行子进程
				_children[i]->Execute(false, false, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
				exit(0);	// 正常运行，返回 0
			} else {
				// 如果是后台进程，我们不需要控制产生的子进程
				// 所以此时需要 fork 两次来避免产生僵死进程
				auto pid = fork();
				if (pid == 0) {
					_children[i]->Execute(false, false, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
					exit(0);
				} else {
					exit(0);
				}
			}
		} else {
			if (is_foreground) {
				setpgid(pid, pid);		// 将子进程加到单独的一个进程组中
				job_pool->AddJob(pid, _children[i]->GetSentence());
			} else {
				waitpid(pid, nullptr, 0);
			}
		}
	}
	_children[size - 1]->Execute(is_shell, true, infile, outfile, errfile);
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

std::string ParaNode::GetOperator() {
	return "&";
}

// 顺序节点
void SeqNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	for (int i = 0; i < size; i++) {
		_children[i]->Execute(is_shell, true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	}
	_children[size - 1]->Execute(is_shell, true, infile, outfile, errfile);
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

std::string SeqNode::GetOperator() {
	return ";";
}

// And 节点
void AndNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	_children[0]->Execute(is_shell, true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	auto globals = SpecialVarPool::Instance();
	for (int i = 1; i < size - 1; i++) {
		if (globals->GetReturn() != 0) {
			break;
		}
		_children[i]->Execute(is_shell, true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	}
	if (globals->GetReturn() == 0) {
		_children[size - 1]->Execute(is_shell, true, infile, outfile, errfile);
	}
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

std::string AndNode::GetOperator() {
	return "&&";
}

// Or 节点
void OrNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	int size = _children.size();
	_children[0]->Execute(is_shell, true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	auto globals = SpecialVarPool::Instance();
	for (int i = 1; i < size - 1; i++) {
		if (globals->GetReturn() == 0) {
			break;
		}
		_children[i]->Execute(is_shell, true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	}
	if (globals->GetReturn() != 0) {
		_children[size - 1]->Execute(is_shell, true, infile, outfile, errfile);
	}
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

std::string OrNode::GetOperator() {
	return "||";
}

// 叶子节点
std::string LeafNode::GetSentence() {
	std::string str(_sentence[0]);
	int size = _sentence.size();
	for (int i = 1; i < size; i++) {
		// 逐步拼合命令
		str += " " + _sentence[i];
	}
	return str;
}

void LeafNode::Print(int step) {
	Node::Print(step);
	fprintf(stdout, "Command: ");
	for (const auto &word : _sentence) {
		fprintf(stdout, "%s ", word.c_str());
	}
	fputc('\n', stdout);
}

void LeafNode::SetSentence(const Sentence &sentence) {
	_sentence = sentence;
}

void LeafNode::Execute(bool is_shell, bool cont,  int infile, int outfile, int errfile) {
	bool is_foreground = tcgetpgrp(STDIN_FILENO) == getpgrp();
	auto jobpool = JobPool::Instance();
	CommandFactory* factory = CommandFactory::Instance();
	auto command = factory->GetCommand(_sentence[0]);
	if (Command::IsExternal(command)) {
		// 如果是外部命令，并且是前台进程。则需要单独开一个进程来做
		auto pid = fork();
		if (pid == 0) {
			// 在子进程中执行外部命令
			pid = getpid();
			if (is_shell) {
				setpgid(pid, pid);
				tcsetpgrp(STDIN_FILENO, pid);
				signal (SIGINT, SIG_DFL);
				signal (SIGTSTP, SIG_DFL);
				signal (SIGQUIT, SIG_DFL);
				signal (SIGTTIN, SIG_DFL);
				signal (SIGTTOU, SIG_DFL);
				signal (SIGCHLD, SIG_DFL);
			}

			command->Execute(_sentence, infile, outfile, errfile);
			fprintf(stderr, "Invalid command: %s\n", _sentence[0].c_str());
			exit(1);	// 如果能运行到这，中间一定出现了问题
		} else {
			if (is_shell) {
				jobpool->AddJob(pid, GetSentence());
				setpgid(pid, pid);
				tcsetpgrp(STDIN_FILENO, pid);
				int status;
				waitpid(pid, &status, WUNTRACED);	// 等待子进程结束
				ReportStatus(pid, status);
				tcsetpgrp(STDIN_FILENO, getpgrp());
			} else {
				waitpid(pid, nullptr, WUNTRACED);
			}
		}
	} else {
		// 如果是内部命令，或者是后台进程，只需要在当前的进程里面执行就好了
		command->Execute(_sentence, infile, outfile, errfile);
	}
	delete command;
	if (cont) {
		return;
	} else {
		exit(0);
	}
}

// 空节点
void NullNode::Print(int step) {
	Node::Print(step);
	fprintf(stdout, "Null Node\n");
}

void NullNode::Execute(bool is_shell, bool cont, int infile, int outfile, int errfile) {
	if (cont) {
		return;
	} else {
		exit(0);
	}
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
		case kSeqNode:
			return new SeqNode;
		case kAndNode:
			return new AndNode;
		case kOrNode:
			return new OrNode;
		default:
			return nullptr;
	}
}

Node *NodeFactory::CreateNullNode() {
	return new NullNode;
}