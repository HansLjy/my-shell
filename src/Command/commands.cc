//
// Created by hansljy on 2021/8/8.
//

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "commands.h"
#include "exceptions.h"

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
	while (arg_id < argc) {
		int redirect = WhichRedirect(args.at(arg_id));
		if (redirect == -1) {	// 重定向没有放在末尾
			fprintf(stderr, "Redirection info should be placed in the end.\n");
			CloseFiles();
			return 1;
		}
		if (arg_id == args.size()) {	// 有重定向标记但没有具体的文件
			fprintf(stderr, "Redirection target missing\n");
			CloseFiles();
			return 1;
		}
		const char* file = args.at(arg_id + 1).c_str();
		switch (redirect) {
			case 0:	// 重载标准输入
				_in = open(file, O_RDONLY | O_CREAT);
				break;
			case 1: // 重载标准输出
				_out = open(file, O_WRONLY | O_CREAT | O_TRUNC);
				break;
			case 2:	// 重载标准错误输出
				_err = open(file, O_WRONLY | O_CREAT | O_TRUNC);
				break;
			case 3:	// 追加重载标准输出
				_out = open(file, O_WRONLY | O_CREAT | O_APPEND);
				break;
			case 4: // 追加重载标准错误输出
				_err = open(file, O_WRONLY | O_CREAT | O_APPEND);
				break;
		}
		arg_id += 2;
	}
	int ret = RealExecute(args);
	CloseFiles();
	return ret;
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

// write 的多态
void write(int fd, const char* str) {
	write(fd, str, strlen(str) * sizeof (char));
}

// Tested
int CommandCd::RealExecute(const Sentence &args) {
	if (args.size() > 2) {
		// cd 的参数至多只能有一个
		write(_err, "cd: too many arguments.\n");
		return 1;
	}
	const char *target = args.size() == 1 ? "~" : args[1].c_str();
	return chdir(target);
}

// Tested
int CommandDir::RealExecute(const Sentence &args) {
	if (args.size() > 2) {
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
	int argc = args.size();
	for (int i = 1; i < argc; i++) {
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
	char* pwd = get_current_dir_name();
	write(_out, pwd);
	write(_out, "\n");
	return 0;
}

// Tested
int CommandTime::RealExecute(const Sentence &args) {
	char buf[100];
	if (args.size() > 1) {
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

// TODO
int CommandClr::RealExecute(const Sentence &args) {
	if (args.size() > 1) {
		// clr 不带参数
		write(_err, "clr: too many arguments.\n");
		return 1;
	}
	for (int i = 1; i < 100; i++)
		write(_out, "\n");	// 直接输出若干空行
	return 0;
}

int CommandExternal::RealExecute(const Sentence &args) {
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
	if (name == "cd") {
		return new CommandCd;
	} else if (name == "dir") {
		return new CommandDir;
	} else if (name == "echo") {
		return new CommandEcho;
	} else if (name == "exit") {
		return new CommandExit;
	} else if (name == "pwd") {
		return new CommandPwd;
	} else if (name == "time") {
		return new CommandTime;
	} else if (name == "clr") {
		return new CommandClr;
	} else {
		return new CommandExternal;
	}
	return nullptr;
}