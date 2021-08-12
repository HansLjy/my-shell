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
protected:
	void CloseFiles();
	int _argc = 0;
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
