//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_COMMANDS_H
#define MYSHELL_COMMANDS_H

#include "parser.h"
#include "tree.h"

class Command {
public:
	/**
	 * 重定向 stdin stdout stderr
	 */
	void Redirect();

	/**
	 * 执行命令
	 * @param args 命令的参数
	 * @param infile 标准输入
	 * @param outfile 标准输出
	 * @param errfile 标准错误输出
	 * @return 命令是否正确完成
	 * @note Execute 采用模板模式进行设计，它只进行所有 Command 共同的操作，
	 * 		 而把真正命令的执行代理给 RealExecute 函数
	 */
	virtual int Execute(const Sentence& args, int infile, int outfile, int errfile);

	/**
	 * 实际上命令的执行
	 * @param args 命令的参数
	 * @return 命令的执行返回值
	 */
	virtual int RealExecute(const Sentence& args) = 0;

	/**
	 * 虚析构函数
	 */
	virtual ~Command() = default;

	/**
	 * 判定当前命令是不是外部命令
	 * @param cmd 待判断的命令对象
	 * @return 是否是外部命令
	 */
	static bool IsExternal(Command* cmd);	// 判断当前命令是不是外部命令

protected:
	/**
	 * 关闭输入输出文件中不是标准输入输出的文件
	 */
	void CloseFiles();

	/**
	 * 备份标准输入输出，因为重定向可能会改变这些输入输出的值
	 */
	void CopyFiles();

	/**
	 * 利用之前的备份还原 标准输入输出
	 */
	void RecoverFiles();

	int _argc = 0; // 包括命令在内的参数个数
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

/**
 * 下面的大部分命令都是重载 RealExecute 函数，我觉得没必要再进行注释了
 * 为了完成五行中一定要有一行注释的标准而去加一些废话我认为是愚蠢的。
 */

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
	/**
	 * 判断参数是不是一个选项
	 * @param str 待判断的参数
	 * @return true 代表是选项
	 */
	bool IsOption(const std::string& str);

	/**
	 * 判断选项具体是哪一个选项
	 * @param str 待判断的选项
	 * @return 选项 id
	 */
	Option WhichOption(const std::string& str);

	/**
	 * 判断 str 对应的文件是不是目录
	 * @param str 待判断的文件名
	 * @return true 代表是目录
	 */
	int CheckDir(const std::string& str);

	/**
	 * 判断 str 对应的文件是不是普通文件
	 * @param str 待判断的文件名
	 * @return true 代表是普通文件
	 */
	int CheckFile(const std::string& str);

	/**
	 * 判断 str 对应的文件是否存在
	 * @param str
	 * @return true 代表存在
	 */
	int CheckExist(const std::string& str);

	/**
	 * 比较 lhs op rhs 是否成立
	 * @param op 比较符号
	 * @return 1 代表 lhs op rhs 成立
	 */
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
	/**
	 * 返回全局唯一的 CommandFactory 对象
	 * @return 全局唯一的 CommandFactory 对象
	 */
	static CommandFactory* Instance();
	Command* GetCommand(const std::string& name);

private:
	static CommandFactory* theFactory;	// 全局唯一的 CommandFactory
};

#endif //MYSHELL_COMMANDS_H
