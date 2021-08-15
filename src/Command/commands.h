//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_COMMANDS_H
#define MYSHELL_COMMANDS_H

#include "parser.h"
#include "tree.h"

class Command {
public:
	void Redirect(int id, int fd);
	virtual int Execute(const Sentence& args);
	virtual int RealExecute(const Sentence& args) = 0;

	virtual ~Command() = default;
	static bool IsExternal(Command* cmd);	// 判断当前命令是不是外部命令
protected:
	void CloseFiles();
	int _argc = 0;						// 包括命令在内的参数个数
	int _in = STDIN_FILENO,				// 默认标准输入
		_out = STDOUT_FILENO,			// 默认标准输出
		_err = STDERR_FILENO;			// 默认标准错误输出
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

class CommandExternal : public Command {
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
