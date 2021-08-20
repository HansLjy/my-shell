//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_DELIMINATOR_H
#define MYSHELL_DELIMINATOR_H

#include <string>

/*
 * Deliminator：文本划分器
 * 简介：将文本按照空格划分成词语
 * 使用方式：使用 str 进行配置，之后不断调用 GetNext 取得一个个分词
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
