#include <iostream>
#include "CLI.h"
#include "exceptions.h"
#include "global.h"
#include <signal.h>

int main(int argc, char* argv[]) {
//	signal (SIGINT, SIG_IGN);
//	signal (SIGQUIT, SIG_IGN);
//	signal (SIGTSTP, SIG_IGN);
//	signal (SIGTTIN, SIG_IGN);
//	signal (SIGTTOU, SIG_IGN);
//	signal (SIGCHLD, SIG_IGN);
	try {
		auto globals = SpecialVarPool::Instance();
		for (int i = argc - 1; i >= 0; i--) {
			globals->SetArg(i, argv[i]);
		}
		CLI *cli = CLI::Instance();
		cli->Initialization();
		cli->Start();
	} catch (Exception exc){
		exc.ShowInfo();
	}
    return 0;
}
