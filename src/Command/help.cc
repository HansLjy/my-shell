//
// Created by hansljy on 2021/8/17.
//

// 这本来就是提示信息了，没必要写注释了吧？？？

#include "help.h"

void HelpInfo::Print() {
	fprintf(stdout, "Usage: %s\nDescription: %s\n", usage.c_str(), description.c_str());
}

static HelpInfo
bg = {
	"bg N",
	"Make job with ID N running in the background"
},
cd = {
	"cd [DIR]",
	"Change the current directory into DIR. If DIR is omitted, change into directory $HOME"
},
clr = {
	"clr",
	"Clear the screen"
},
dir = {
	"dir DIR",
	"List all the files inside directory DIR"
},
echo = {
	"echo STRING",
	"Print STRING on the screen"
},
exec = {
	"exec COMMAND",
	"Replace current process with COMMAND"
},
mexit = {
	"exit",
	"Exit this shell"
},
fg = {
	"fg N",
	"Make job with ID N a foreground job"
},
help = {
	"help [COMMAND]",
	"Display the help info of command COMMAND\nIf COMMAND is not provided, display all."
},
jobs = {
	"jobs",
	"List all the jobs"
},
pwd = {
	"pwd",
	"Display the current working directory"
},
set = {
	"set [VAR1 VAR2 ...]",
	"Display all the environment variables. If VAR is set, the command set $i = VARi"
},
shift = {
	"shift [N]",
	"Left shift positional variables by N. For example $1 will become ${1 + N}. N is by default 1"
},
test = {
	"test [option] FILE | test INT1 [op] INT2",
	"For the first mode:\n"
	"\t-d\tCheck if the file is a directory \n"
	"\t-f\tCheck if the file is a file\n"
	"\t-e\tCheck if the file exists\n"
	"For the second mode:\n"
	"\t-lt\tCheck if INT1 < INT2\n"
	"\t-le\tCheck if INT1 <= INT2\n"
	"\t-eq\tCheck if INT1 == INT2\n"
	"\t-ne\tCheck if INT1 != INT2\n"
	"\t-ge\tCheck if INT1 >= INT2\n"
	"\t-gt\tCheck if INT1 > INT2"
},
mtime = {
	"time",
	"Display current time"
},
mumask = {
	"umask [mask]",
	"If MASK is not provided, display the current mask.\n"
	"Otherwise, set the mask."
},
unset = {
	"unset VAR",
	"Unset environment variable VAR"
};

Helper* Helper::theHelper = nullptr;

Helper *Helper::Instance() {
	if (theHelper == nullptr) {
		theHelper = new Helper;
	}
	return theHelper;
}

void Helper::PrintHelp(const std::string &name) {
	if (name == "bg") {
		bg.Print();
	} else if (name == "cd") {
		cd.Print();
	} else if (name == "clr") {
		clr.Print();
	} else if (name == "dir") {
		dir.Print();
	} else if (name == "echo") {
		echo.Print();
	} else if (name == "exec") {
		exec.Print();
	} else if (name == "exit") {
		mexit.Print();
	} else if (name == "fg") {
		fg.Print();
	} else if (name == "help") {
		help.Print();
	} else if (name == "jobs") {
		jobs.Print();
	} else if (name == "pwd") {
		pwd.Print();
	} else if (name == "set") {
		set.Print();
	} else if (name == "shift") {
		shift.Print();
	} else if (name == "test") {
		test.Print();
	} else if (name == "time") {
		mtime.Print();
	} else if (name == "umask") {
		mumask.Print();
	} else if (name == "unset") {
		unset.Print();
	} else {
		fprintf(stdout, "%s is an external command. Use 'man %s' to find out more.\n", name.c_str(), name.c_str());
	}
}

void Helper::PrintAll() {
	fprintf(stdout, "COMMAND bg:\n");
	bg.Print();
	fprintf(stdout, "\nCOMMAND cd:\n");
	cd.Print();
	fprintf(stdout, "\nCOMMAND clr:\n");
	clr.Print();
	fprintf(stdout, "\nCOMMAND dir:\n");
	dir.Print();
	fprintf(stdout, "\nCOMMAND echo:\n");
	echo.Print();
	fprintf(stdout, "\nCOMMAND exec:\n");
	exec.Print();
	fprintf(stdout, "\nCOMMAND exit:\n");
	mexit.Print();
	fprintf(stdout, "\nCOMMAND fg:\n");
	fg.Print();
	fprintf(stdout, "\nCOMMAND help:\n");
	help.Print();
	fprintf(stdout, "\nCOMMAND jobs:\n");
	jobs.Print();
	fprintf(stdout, "\nCOMMAND pwd:\n");
	pwd.Print();
	fprintf(stdout, "\nCOMMAND set:\n");
	set.Print();
	fprintf(stdout, "\nCOMMAND shift:\n");
	shift.Print();
	fprintf(stdout, "\nCOMMAND test:\n");
	test.Print();
	fprintf(stdout, "\nCOMMAND time:\n");
	mtime.Print();
	fprintf(stdout, "\nCOMMAND umask:\n");
	mumask.Print();
	fprintf(stdout, "\nCOMMAND unset:\n");
	unset.Print();
}
