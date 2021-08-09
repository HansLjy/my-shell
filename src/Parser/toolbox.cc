//
// Created by hansljy on 2021/8/8.
//

#include <cstring>
#include <cctype>
#include "toolbox.h"

ToolBox* ToolBox::theToolBox = nullptr;	// 使用时再初始化，避免初始化之间的依赖，虽然这里没有

ToolBox *ToolBox::Instance() {
	if (theToolBox == nullptr) {	// 如果还没有使用过
		theToolBox = new ToolBox;	// 新建一个
	}
	return theToolBox;
}
