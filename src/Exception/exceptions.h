//
// Created by hansljy on 2021/8/9.
//

#ifndef MYSHELL_EXCEPTIONS_H
#define MYSHELL_EXCEPTIONS_H

#include <string>

class Exception {
public:
	/**
	 * Exception 构造函数
	 * @param str 异常对应的提示信息
	 */
	Exception(const std::string& str);

	/**
	 * 打印 Exception 信息
	 */
	virtual void ShowInfo();

protected:
	std::string _info;	// 异常对应的提示信息
};

#endif //MYSHELL_EXCEPTIONS_H
