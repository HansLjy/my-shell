//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_DELIMINATOR_H
#define MYSHELL_DELIMINATOR_H

#include <string>

/*
 * Deliminator：文本划分器
 * 简介：将文本按照空格划分成词语
 * 警告：
 * 	1. 一旦使用字符串初始化
 */
class Deliminator {
public:
	Deliminator(char* str);
	std::string GetNext();	// 取得下一个标记
	~Deliminator();

private:
	bool IsDeliminator(char ch);

	char* _str;			// 待划分的字符串
	const int _len;		// 待划分的字符串的长度
	int _cur;			// 当前遍历到的位置
};

#endif //MYSHELL_DELIMINATOR_H
