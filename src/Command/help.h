//
// Created by hansljy on 2021/8/17.
//

#ifndef MYSHELL_HELP_H
#define MYSHELL_HELP_H

#include <string>

struct HelpInfo {
	std::string usage;
	std::string description;
	void Print();
};

class Helper {
public:
	static Helper *Instance();
	void PrintHelp(const std::string& str);
	void PrintAll();

private:
	static Helper* theHelper;
};

#endif //MYSHELL_HELP_H
