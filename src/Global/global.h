//
// Created by hansljy on 2021/8/15.
//

#ifndef MYSHELL_GLOBAL_H
#define MYSHELL_GLOBAL_H

#include <vector>
#include <string>

/*
 * 特殊变量池，含义：
 * $0 ~ $N 参数变量
 * $? 上一条指令的返回值
 * $# 总参数个数
 */

class SpecialVarPool {
public:
	static SpecialVarPool* Instance();

	/**
	 * 设置参数
	 * @param idx 参数的标号
	 * @param val 参数的值
	 * @note 即 $idx = val
	 */
	void SetArg(int idx, const std::string& val);

	/**
	 * 获取参数
	 * @param idx 参数的编号
	 * @return 参数的值
	 */
	std::string GetArg(int idx);

	/**
	 * 获得参数个数
	 * @return 参数个数
	 * @note 参数个数只增不减
	 */
	int GetArgc();

	/**
	 * 设置返回值
	 * @param ret 返回值
	 * @note 任何子程序在运行结束后都应当设置这个返回值
	 */
	void SetReturn(int ret);

	/**
	 * 获得返回值
	 * @return （上一个子程序的）返回值
	 */
	int GetReturn();

	/**
	 * 移动参数
	 * @param n 移动的距离，$1 + n 被移动到 $1
	 * @return 如果 n 超过范围，返回 false
	 * @note
	 * 	1. 如果超出范围，不进行实际操作
	 * 	2. 移动从 $1 开始，$0 不参与移动
	 */
	bool Shift(int n);

	/**
	 * 清空参数表
	 */
	void Clear();									// 清空参数表

private:
	std::vector<std::string> _args;		// 环境变量 $i
	int _ret = 0;						// 返回值
	static SpecialVarPool* thePool;
};

#endif //MYSHELL_GLOBAL_H
