//
// Created by hansljy on 2021/8/15.
//

#ifndef MYSHELL_JOBPOOL_H
#define MYSHELL_JOBPOOL_H

class JobPool {
public:
	static JobPool* Instance();

private:
	static JobPool* thePool;
};

#endif //MYSHELL_JOBPOOL_H
