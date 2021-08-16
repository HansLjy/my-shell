//
// Created by hansljy on 2021/8/6.
//

#ifndef MYSHELL_CLI_H
#define MYSHELL_CLI_H

#include <string>

class CLI {
public:
    static CLI* Instance();
    void Initialization();

	void Start();
private:
    static CLI* theCLI;
    std::string prompt;
};

#endif //MYSHELL_CLI_H
