//
// Created by hansljy on 2021/8/9.
//

#include "exceptions.h"

Exception::Exception(const std::string& str) {
	_info = str;
}

void Exception::ShowInfo() {
	// 打印 Exception 信息
	fprintf(stderr, _info.c_str());
	fputc('\n', stderr);
}