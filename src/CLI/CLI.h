//
// Created by hansljy on 2021/8/6.
//

#ifndef MYSHELL_CLI_H
#define MYSHELL_CLI_H

#include <string>

class CLI {
public:
	/**
	 * 返回全局唯一的 CLI 对象
	 * @return 全局唯一的 CLI 对象
	 */
    static CLI* Instance();

    /**
     * CLI 初始化
     */
    void Initialization();

    /**
     * CLI 开始运行
     * @param is_shell 是否以 shell 交互模式运行，若此项为 false，不显示 prompt 以及作业控制信息
     */
	void Start(bool is_shell);
private:
    static CLI* theCLI;	// 全局唯一 CLI 对象
};

#endif //MYSHELL_CLI_H
