//
// Created by hansljy on 2021/8/6.
//

#include <unistd.h>
#include <stdlib.h>
#include <cstring>

#include "CLI.h"
#include "parser.h"

CLI* CLI::theCLI = nullptr;

CLI *CLI::Instance() {
    if (theCLI == nullptr) {    // 如果还没有初始化过
        theCLI = new CLI;       // 初始化
        return theCLI;          // 返回新的终端对象
    }
    return theCLI;              // 否则就返回全局唯一的 CLI 对象
}

void CLI::Initialization() {
	auto parser = Parser::Instance();
	parser->AddOperator(Operator{
		Token("|"), 1, kPipedNode
	});
	parser->AddOperator(Operator{
		Token ("&"), 2, kParaNode
	});
}

void CLI::Start() {
	srand(time(NULL));
	auto parser = Parser::Instance();
	while (true) {
		prompt = getlogin();	// 获取用户名
		prompt = prompt + ":";	// 分隔用户名和路径
		prompt = prompt + get_current_dir_name();
		char *line = nullptr;
		size_t len = 0;
		fprintf(stdout, "%s$", prompt.c_str());
		getline(&line, &len, stdin);
		line[strlen(line) - 1] = '\0';	// 替换掉行末的空格
		if (strlen(line) == 0) {
			break;
		}
        auto res = parser->Parse(line);
		res->Print(2);
		res->Execute(true, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
		delete res;
        free(line);
    }
}