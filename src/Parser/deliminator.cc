//
// Created by hansljy on 2021/8/8.
//

#include "deliminator.h"
#include <cstring>

Deliminator::Deliminator(char *str) : _cur(0), _len(strlen(str)) {
	_str = new char[_len + 1];
	strcpy(_str, str);
}

bool Deliminator::IsDeliminator(char ch) {
	return ch == '(' || ch == ')' || ch == ';';
}

std::string Deliminator::GetNext() {
	// 找到第一个非空格字符为止
	while (_cur < _len && isspace(_str[_cur])) {
		_cur++;
	}
	if (_cur >= _len) {	// 如果已经没有非空字符了
		return std::string();	// 返回空字符串
	}
	if (IsDeliminator(_str[_cur])) {	// 如果是括号
		return std::string (1, _str[_cur++]);	// 直接插入
	}
	int start = _cur;	// 标记开始的位置
	// 一直找到第一个分隔字符（空白字符、括号）为止
	while (_cur < _len && !isspace(_str[_cur]) && !IsDeliminator(_str[_cur])) {
		_cur++;
	}
	if (_cur >= _len) {	// 如果已经提取到最后一个元素了
		return std::string(_str + start);
	} else {
		char tmp = _str[_cur];	// 结尾的字符
		_str[_cur] = '\0';
		std::string result(_str + start);
		_str[_cur] = tmp;
		return result;
	}
}

Deliminator::~Deliminator() {
	delete [] _str;
}