//
// Created by hansljy on 2021/8/9.
//

#include "exceptions.h"

Exception::Exception(const std::string& str) {
	_info = str;
}

void Exception::ShowInfo() {
	fprintf(stderr, _info.c_str());
	fputc('\n', stderr);
}