//
// Created by hansljy on 2021/8/8.
//

#ifndef MYSHELL_TOOLBOX_H
#define MYSHELL_TOOLBOX_H

#include <string>

typedef std::string Token;

class ToolBox {
public:
	static ToolBox* Instance();

private:
	static ToolBox* theToolBox;
};

#endif //MYSHELL_TOOLBOX_H
