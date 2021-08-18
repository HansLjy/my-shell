//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_COMMANDS_H
#define MYSHELL_COMMANDS_H

#include "parser.h"
#include "tree.h"

class Command {
public:
	void Redirect();
	virtual int Execute(const Sentence& args, int infile, int outfile, int errfile);
	virtual int RealExecute(const Sentence& args) = 0;

	virtual ~Command() = default;
	static bool IsExternal(Command* cmd);	// 判断当前命令是不是外部命令
protected:
	void CloseFiles();
	void CopyFiles();					// 保存原来的输入输出文件
	void RecoverFiles();				// 恢复原来的输入输出文件
	int _argc = 0;						// 包括命令在内的参数个数
	/*
	 * 简单解释一下这里为什么有三组输入输出：
	 * 1. _stdxx 系列是用来保存原来的输入输出的，因为涉及到外部指令
	 *    不可避免得要直接重定向，为了保证命令执行完毕之后我们还能够
	 *    回到原来的输入输出，必须要拷贝一份原来的标准 IO
	 * 2. _xx 系列是用来保存在不考虑单条指令的重定向情况下该指令的
	 *    输入输出，比方说 cat test.txt | grep X 这条命令中前一条
	 *    指令应该具有的输出是一个管道文件
	 * 3. _xx_red 系列用来保存这条指令本身的重定向信息，比方说这条指
	 *    令可能是 man cd > test.txt ，此时 _our_red 就应该是一
	 *    个文件的 file descriptor
	 */
	int _in = STDIN_FILENO,				// 默认标准输入
		_out = STDOUT_FILENO,			// 默认标准输出
		_err = STDERR_FILENO;			// 默认标准错误输出
	int _in_red = STDIN_FILENO,			// 重定向输入
		_out_red = STDOUT_FILENO,		// 重定向输出
		_err_red = STDERR_FILENO;		// 重定向错误输出
	int _stdin, _stdout, _stderr;		// 标准输入输出文件的拷贝
};

class CommandCd : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandDir : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandEcho : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandExit : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandPwd : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandTime : public  Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandClr : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandExec : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandSet : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandUnset : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandShift : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandTest : public Command {
public:
	virtual int RealExecute(const Sentence& args);

private:
	enum Option {
		kInvalid,	// Invalid options
		kDir,		// -d
		kFile,		// -f
		kExist,		// -e
		kEq,		// -eq
		kGe,		// -ge
		kGt,		// -gt
		kLe,		// -le
		kLt,		// -lt
		kNe,		// -ne
	};
	bool IsOption(const std::string& str);
	Option WhichOption(const std::string& str);

	int CheckDir(const std::string& str);
	int CheckFile(const std::string& str);
	int CheckExist(const std::string& str);
	int Compare(const Option& op, int lhs, int rhs);
};

class CommandUmask : public Command {
public:
	virtual int RealExecute(const Sentence& args);

};

class CommandExternal : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandJobs : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandFg : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandBg : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandHelp : public Command {
public:
	virtual int RealExecute(const Sentence& args);
};

class CommandFactory {
public:
	static CommandFactory* Instance();
	Command* GetCommand(const std::string& name);

private:
	static CommandFactory* theFactory;
};

#endif //MYSHELL_COMMANDS_H
