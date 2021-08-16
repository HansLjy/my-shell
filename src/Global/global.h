//
// Created by hansljy on 2021/8/15.
//

#ifndef MYSHELL_GLOBAL_H
#define MYSHELL_GLOBAL_H

#include <vector>
#include <string>

/*
 * 特殊变量池，含义：
 * $0 ~ $9 参数变量（可以支持更多，但是多出来的没法打印出来，因为我实在是懒得写了）
 * $? 上一条指令的返回值
 */

class SpecialVarPool {
public:
	static SpecialVarPool* Instance();

	void SetArg(int idx, const std::string& val);	// 设置参数变量的值
	std::string GetArg(int idx);					// 获取参数变量的值（超过限制返回空串）
	int GetArgc();									// 获取参数个数
	void SetReturn(int ret);						// 设置程序的返回值
	int GetReturn();								// 获取程序的返回值
	bool Shift(int n);								// 移动参数的值
	void Clear();									// 清空参数表

private:
	std::vector<std::string> _args;		// 环境变量 $i
	int _ret = 0;						// 返回值
	static SpecialVarPool* thePool;
};

#endif //MYSHELL_GLOBAL_H
