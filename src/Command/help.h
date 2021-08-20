//
// Created by hansljy on 2021/8/17.
//

#ifndef MYSHELL_HELP_H
#define MYSHELL_HELP_H

#include <string>

struct HelpInfo {
	std::string usage;			// 用法
	std::string description;	// 描述
	void Print();				// 打印 HelpInfo
};

class Helper {
public:
	/**
	 * 返回全局唯一的 Helper 对象
	 * @return 全局唯一的 Helper 对象
	 */
	static Helper *Instance();

	/**
	 * 打印帮助信息
	 * @param str 待打印帮助信息的命令
	 */
	void PrintHelp(const std::string& str);

	/**
	 * 打印所有帮助信息
	 */
	void PrintAll();

private:
	static Helper* theHelper;	// 全局唯一的 Helper 对象
};

#endif //MYSHELL_HELP_H
