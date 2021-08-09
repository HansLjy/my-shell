#include <iostream>
#include "CLI.h"
#include "exceptions.h"

int main() {
	try {
		CLI *cli = CLI::Instance();
		cli->Initialization();
		cli->Start();
	} catch (Exception exc){
		exc.ShowInfo();
	}
    return 0;
}
