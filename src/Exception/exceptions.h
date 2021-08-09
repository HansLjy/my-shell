//
// Created by hansljy on 2021/8/9.
//

#ifndef MYSHELL_EXCEPTIONS_H
#define MYSHELL_EXCEPTIONS_H

#include <string>

class Exception {
public:
	Exception(const std::string& str);
	virtual void ShowInfo();

protected:
	std::string _info;
};

#endif //MYSHELL_EXCEPTIONS_H
