//
// Created by hansljy on 2021/8/8.
//

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <cstdlib>
#include "commands.h"
#include "exceptions.h"
#include "global.h"
#include "jobpool.h"

// 判断是哪一个输入输出的重定向
int WhichRedirect(const std::string& arg) {
	if (arg == "<" || arg == "0<")		// 重定向标准输入
		return 0;
	if (arg == ">" || arg == "1>")		// 重定向标准输出
		return 1;
	if (arg == "2>")					// 重定向标准错误输出
		return 2;
	if (arg == ">>" || arg == "1>>")	// 追加重载标准输出
		return 3;
	if (arg == "2>>")					// 追加重载标准错误输出
		return 4;
	return -1;							// 不是重定向符号
}

void Command::Redirect(int id, int fd) {
	switch (id) {
		case 0: _in = fd; break;
		case 1: _out = fd; break;
		case 2: _err = fd; break;
	}
}

// 拷贝输入输出文件
void Command::CopyFiles() {
	_stdin = dup(STDIN_FILENO);
	_stdout = dup(STDOUT_FILENO);
	_stderr = dup(STDERR_FILENO);
}

// 恢复标准输入输出文件
void Command::RecoverFiles() {
	dup2(_stdin, STDIN_FILENO);
	dup2(_stdout, STDOUT_FILENO);
	dup2(_stderr, STDERR_FILENO);
}

// 关闭所有重定向之后的文件
void Command::CloseFiles() {
	if (_in != STDIN_FILENO)
		close(_in);
	if (_out != STDOUT_FILENO)
		close(_out);
	if (_err != STDERR_FILENO)
		close(_err);
}

/*
 * 基类的 Execute 主要负责如下工作：
 * 1. 检查重定向符号，进行重定向，打开相关文件
 * 完成这些工作后，调用 RealExecute 进行子类的调用真正执行命令
 */
int Command::Execute(const Sentence &args) {
	int argc = args.size();
	int arg_id = 1;
	while (arg_id < argc && WhichRedirect(args.at(arg_id)) == -1)
		arg_id++;
	_argc = arg_id;	// 参数到此为止
	// 以下都是重定向信息
	CopyFiles();
	while (arg_id < argc) {
		int redirect = WhichRedirect(args.at(arg_id));
		if (redirect == -1) {	// 重定向没有放在末尾
			fprintf(stderr, "Redirection info should be placed in the end.\n");
			CloseFiles();		// 关闭已经打开的文件
			RecoverFiles();		// 恢复正常的输入输出
			return 1;
		}
		if (arg_id == args.size()) {	// 有重定向标记但没有具体的文件
			fprintf(stderr, "Redirection target missing\n");
			CloseFiles();		// 关闭已经打开的文件
			RecoverFiles();		// 恢复正常的输入输出
			return 1;
		}
		const char* file = args.at(arg_id + 1).c_str();
		switch (redirect) {
			case 0:	// 重载标准输入
				_in = open(file, O_RDONLY | O_CREAT, 0666);
				break;
			case 1: // 重载标准输出
				_out = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				break;
			case 2:	// 重载标准错误输出
				_err = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				break;
			case 3:	// 追加重载标准输出
				_out = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
				break;
			case 4: // 追加重载标准错误输出
				_err = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
				break;
		}
		arg_id += 2;
	}
	dup2(_in, STDIN_FILENO);
	dup2(_out, STDOUT_FILENO);
	dup2(_err, STDERR_FILENO);
	int ret = RealExecute(args);
	CloseFiles();
	RecoverFiles();
	SpecialVarPool::Instance()->SetReturn(ret);
	return ret;
}

bool Command::IsExternal(Command *cmd) {
	if (dynamic_cast<CommandExternal*>(cmd) == nullptr) {
		// 动态类型转换，当转换失败的时候说明不是外部命令
		return false;
	} else {
		return true;
	}
}

// write 的多态
void write(int fd, const char* str) {
	write(fd, str, strlen(str) * sizeof (char));
}

// Tested
int CommandCd::RealExecute(const Sentence &args) {
	if (_argc > 2) {
		// cd 的参数至多只能有一个
		write(_err, "cd: too many arguments.\n");
		return 1;
	}
	const char *target = args.size() == 1 ? "~" : args[1].c_str();
	return chdir(target);
}

// Tested
int CommandDir::RealExecute(const Sentence &args) {
	if (_argc > 2) {
		// dir 的参数至多只能有 1 个
		write(_err, "dir: too many arguments.\n");
		return 1;
	}
	const char * target = args.size() == 1 ? "." : args[1].c_str();
	auto dir = opendir(target);	// 打开目标文件夹
	if (dir == nullptr) {
		write(_err, "dir: directory does not exist!\n");
		return 1;
	}
	auto entry = readdir(dir);	// 一个个取出文件读取
	while (entry != nullptr) {
		write(_out, entry->d_name);
		write(_out, "\n");
		entry = readdir(dir);
	}
	return 0;
}

// Tested
int CommandEcho::RealExecute(const Sentence &args) {
	for (int i = 1; i < _argc; i++) {
		write(_out, args[i].c_str());
		write(_out, " ");
	}
	write(_out, "\n");
	return 0;
}

// Tested
int CommandExit::RealExecute(const Sentence &args) {
	exit(0);
}

// Tested
int CommandPwd::RealExecute(const Sentence &args) {
	if (_argc > 1) {
		write(_err, "pwd: too many arguments.\n");
		return 1;
	}
	char* pwd = get_current_dir_name();
	write(_out, pwd);
	write(_out, "\n");
	return 0;
}

// Tested
int CommandTime::RealExecute(const Sentence &args) {
	char buf[100];
	if (_argc > 1) {
		// Time 不能含有参数
		write(_err, "time: too many arguments.\n");
		return 1;
	}
	time_t cur_time;
	time(&cur_time);	// 获得当前时间参数
	tm* info = localtime(&cur_time);
	sprintf(buf, "%d.%d.%d %d:%d:%d\n",
			info->tm_year + 1900, info->tm_mon + 1, info->tm_mday,	// 年月日
			info->tm_hour, info->tm_min, info->tm_sec				// 时分秒
			);
	write(_out, buf);
	return 0;
}

// Tested
// 注意 CLion 自带的窗口由于是只读的所以会有问题
int CommandClr::RealExecute(const Sentence &args) {
	if (_argc > 1) {
		// clr 不带参数
		write(_err, "clr: too many arguments.\n");
		return 1;
	}
	write(_out, "\x1b[H\x1b[2J");	// 输出清屏用的特殊字符串
	return 0;
}

// Tested
int CommandExec::RealExecute(const Sentence &args) {
	if (_argc < 2) {
		write(_err, "exec: too few arguments\n");
		return 1;
	}
	auto command = CommandFactory::Instance()->GetCommand(args[1].c_str());
	Sentence new_args;
	for (int i = 1; i < _argc; i++) {
		new_args.push_back(args[i]);
	}
	exit(command->Execute(new_args));
}

// Tested
int CommandSet::RealExecute(const Sentence &args) {
	if (_argc > 1) {
		// 设置环境变量
		auto globals = SpecialVarPool::Instance();
		for (int i = _argc - 1; i >= 1; i--) {
			globals->SetArg(i, args[i].c_str());
		}
	} else {
		extern char** environ;	// 环境变量数组
		for (int index = 0; environ[index] != nullptr; index++) {
			// 逐个打印
			write(_out, environ[index]);
			write(_out, "\n");
		}
	}
	return 0;
}

// Tested
int CommandUnset::RealExecute(const Sentence &args) {
	if (_argc < 2) {
		write(_err, "unset: too few arguments\n");
		return 1;
	}
	for (int i = 1; i < _argc; i++) {
		unsetenv(args[i].c_str());
	}
	return 0;
}

// Tested
int CommandShift::RealExecute(const Sentence &args) {
	auto globals = SpecialVarPool::Instance();
	if (args.size() > 2) {
		write(_err, "shift: too many arguments\n");
		globals->SetReturn(1);
		return 1;
	}
	int shift = 1; // 默认参数为 1
	if (args.size() == 2) {
		shift = atoi(args[1].c_str());	// 不检查错误了
	}
	int success = globals->Shift(shift);
	if (!success) {
		write(_err, "shift: n should be in range [0, argc]\n");
		return 1;
	}
	return 0;
}

// 判断是不是选项
bool CommandTest::IsOption(const std::string &str) {
	return str[0] == '-';
}

// 判断是哪一个参数
// 我想下面的代码应该不需要注释了吧……
typename CommandTest::Option CommandTest::WhichOption(const std::string &str) {
	if (str == "-d")	return kDir;
	if (str == "-f")	return kFile;
	if (str == "-e")	return kExist;
	if (str == "-eq")	return kEq;
	if (str == "-ge")	return kGe;
	if (str == "-gt")	return kGt;
	if (str == "-le")	return kLe;
	if (str == "-lt")	return kLt;
	if (str == "-ne")	return kNe;
	return kInvalid;
}

// 比较两个 int
// 下面的代码就不需要注释了吧……
int CommandTest::Compare(const Option &op, int lhs, int rhs) {
	switch (op) {
		case kEq:	return lhs == rhs;
		case kGe:	return lhs >= rhs;
		case kGt:	return lhs > rhs;
		case kLe:	return lhs <= rhs;
		case kLt:	return lhs < rhs;
		case kNe:	return rhs != rhs;
	}
	return true;
}

// 检查是不是文件
int CommandTest::CheckFile(const std::string &str) {
	struct stat stat_buffer;					// 用来暂存 stat
	stat(str.c_str(), &stat_buffer);			// 获得文件信息
	return S_ISREG(stat_buffer.st_mode);	// 判断是不是文件
}

// 检查是不是目录
int CommandTest::CheckDir(const std::string &str) {
	struct stat stat_buffer;					// 用来暂存 stat
	stat(str.c_str(), &stat_buffer);			// 获得文件信息
	return S_ISDIR(stat_buffer.st_mode);	// 判断是不是目录
}

// 检查文件是否存在
int CommandTest::CheckExist(const std::string &str) {
	return !access(str.c_str(), F_OK);	// 检查文件是否存在
}

// Tested
/*
 * Note：
 * test 相对来说是所有命令里面最麻烦的了，原因是如果要实现
 * 一模一样的功能几乎需要重新写一个 parser，并且这个 parser
 * 几乎不能复用，所以这里就没这么做了，目前只实现了部分功能。
 * 好在现在这个项目的可 * 扩展性还行，如果以后有空了或者开源
 * 社区的老铁有兴趣的话可能会扩展 test 的功能吧。
 */
int CommandTest::RealExecute(const Sentence &args) {
	if (_argc < 2) {
		write(_err, "test: too few arguments\n");
		return 1;
	}
	if (IsOption(args[1])) {
		auto op = WhichOption(args[1]);
		if (_argc != 3) {
			// 此时只可能是 test -d file 这样的形式
			write(_err, "test: too many or too few arguments\n");
			return 1;
		}
		switch (op) {
			case kDir: 		return !CheckDir(args[2]);
			case kFile: 	return !CheckFile(args[2]);
			case kExist:	return !CheckExist(args[2]);
			default:
				write(_err, "test: invalid option");
				return 1;
		}
		// -d -f -e 三种情况
	} else {
		if (_argc != 4) {
			write(_err, "test: too many or too few arguments\n");
		}
		auto op = WhichOption(args[2]);
		int lhs = atoi(args[1].c_str());
		int rhs = atoi(args[3].c_str());
		switch (op) {
			case kEq:
			case kGe:
			case kGt:
			case kLe:
			case kLt:
			case kNe:
				return !Compare(op, lhs, rhs);
			default:
				write(_err, "test: invalid option");
				return 1;
		}
	}
}

// Tested
int CommandUmask::RealExecute(const Sentence &args) {
	if (_argc > 2) {
		write(_err, "umask: too many arguments\n");
		return 1;
	}
	if (_argc == 1) {
		// 查询 umask
		auto mask = umask(0);	// 通过修改的方式获得之前的 mask
		umask(mask);				// 改回去
		char buffer[10];
		sprintf(buffer, "%04o", mask);	// 调整格式
		write(_out, buffer);
		write(_out, "\n");
		return 0;
	} else {
		// 改变 umask
		mode_t mode;
		char *end;
		mode = strtoul(args[1].c_str(), &end, 8);
		if (*end != '\0') {
			write(_err, "umask: invalid mask\n");
			return 1;
		}
		umask(mode);
		return 0;
	}
}

// Tested
// 执行外部命令
int CommandExternal::RealExecute(const Sentence &args) {
	auto p_args = new char*[_argc + 1];	// 用来拷贝 execvp 的参数表
	for (int i = 0; i < _argc; i++) {
		// 将 args[i] 的内容拷贝到 p_args[i] 中
		p_args[i] = new char[args[i].size() + 1];
		strcpy(p_args[i], args[i].c_str());
	}
	p_args[_argc] = nullptr;
	execvp(args[0].c_str(), p_args);
	for (int i = 0; i < _argc; i++) {
		delete [] p_args[i];
	}
	delete [] p_args;
	return 1;
}

int CommandJobs::RealExecute(const Sentence &args) {
	auto job_pool = JobPool::Instance();
	job_pool->PrintJobs();
	return 0;
}

int CommandFg::RealExecute(const Sentence &args) {
	bool is_foreground = tcgetpgrp(_stdin) == getpgrp();	// 是不是前端运行
	bool is_interactive = isatty(STDIN_FILENO);				// 是不是交互式 shell
	if (is_foreground && is_interactive) {
		// 只有当前端运行且是交互式 shell 的时候才会执行 fg 命令
		if (_argc != 2) {
			fprintf(stderr, "fg: too many or too few arguments\n");
			return 1;
		}
		int id = strtoul(args[1].c_str(), nullptr, 10);
		auto job_pool = JobPool::Instance();
		pid_t pid = job_pool->GetPID(id);
		tcsetpgrp(STDIN_FILENO, pid);
		waitpid(pid, nullptr, 0);
		printf("%d\n", getpgrp());
		tcsetpgrp(STDIN_FILENO, getpgrp());
	}
	return 0;
}

CommandFactory* CommandFactory::theFactory = nullptr;

CommandFactory *CommandFactory::Instance() {
	if (theFactory == nullptr) {
		theFactory = new CommandFactory;
	}
	return theFactory;
}

Command *CommandFactory::GetCommand(const std::string &name) {
	if (name == "cd") return new CommandCd;
	if (name == "dir") return new CommandDir;
	if (name == "echo") return new CommandEcho;
	if (name == "exit") return new CommandExit;
	if (name == "pwd") return new CommandPwd;
	if (name == "time") return new CommandTime;
	if (name == "clr") return new CommandClr;
	if (name == "exec") return new CommandExec;
	if (name == "set") return new CommandSet;
	if (name == "unset") return new CommandUnset;
	if (name == "shift") return new CommandShift;
	if (name == "test") return new CommandTest;
	if (name == "umask") return new CommandUmask;
	if (name == "jobs") return new CommandJobs;
	if (name == "fg") return new CommandFg;
	return new CommandExternal;
}