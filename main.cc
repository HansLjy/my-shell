#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include "CLI.h"
#include "exceptions.h"
#include "global.h"
#include "jobpool.h"

void Init() {
	static char str[300] = "SHELL=";	// putenv 需要使用 str,所以不能是 auto 型的
	int shell_terminal = STDIN_FILENO;
	int shell_is_interactive = isatty (shell_terminal);					// 是否是交互式的shell
	int shell_pgid;

	if (shell_is_interactive) {
		while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))	// 是否是前台进程
			kill (-shell_pgid, SIGTTIN);

		// 忽略各种挂断信号，防止 C-c 使得程序退出
		signal (SIGINT, SIG_IGN);
		signal (SIGQUIT, SIG_IGN);
		signal (SIGTSTP, SIG_IGN);
		signal (SIGTTIN, SIG_IGN);
		signal (SIGTTOU, SIG_IGN);
		signal (SIGCHLD, CHLDHandler);

		// 为自己开辟一个新的进程组
		shell_pgid = getpid ();
		if (setpgid (shell_pgid, shell_pgid) < 0) {
			perror ("Couldn't put the shell in its own process group");
			exit (1);
		}

		// 控制终端
		tcsetpgrp (shell_terminal, shell_pgid);
	}
	char *p;
	readlink("/proc/self/exe", str + strlen(str), 300);
	if ((p = strrchr(str,'/')) != NULL)
		*p = '\0';
	strcat(str, "/MyShell");
	putenv(str);
}

int main(int argc, char* argv[]) {
	auto globals = SpecialVarPool::Instance();
	for (int i = argc - 1; i >= 0; i--) {
		globals->SetArg(i, argv[i]);
	}
	Init();
	CLI *cli = CLI::Instance();
	cli->Initialization();
	if (argc == 1) {
		cli->Start(true);
	} else {
		freopen(argv[1], "r", stdin);
		cli->Start(false);
	}
    return 0;
}
