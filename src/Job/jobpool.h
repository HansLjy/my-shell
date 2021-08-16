//
// Created by hansljy on 2021/8/15.
//

#ifndef MYSHELL_JOBPOOL_H
#define MYSHELL_JOBPOOL_H

#include <unistd.h>
#include <string>
#include <vector>
#include <queue>

struct Job {
	enum Status {
		kDone,				// 作业已经完成
		kKilled,			// 作业被杀死
		kRunning,			// 作业正在运行
		kSuspended,			// 作业被挂起
	} _status;
	pid_t _pid;		// 作业的 pid
	std::string _str;	// 描述作业用的字符串
	Job(Status status, pid_t pid, const std::string& str);
};

class JobPool {
public:
	static JobPool* Instance();
	int AddJob(pid_t pid, const std::string& str);		// 新加入一个作业
	int GetPID(int idx);								// 通过 id 获得 pid
	void ChangeState(pid_t pgid, Job::Status status);	// 改变作业的状态
	void PrintJobs();									// 打印目前还在运行或者被挂起的作业
	void PrintFinished();								// 打印已经完成的所有作业

	friend void CHLDHandler (int sig);

private:
	std::vector<Job> _jobs;
	std::queue<Job> _finished_jobs;
	static JobPool* thePool;
};

void CHLDHandler (int sig);

void STGHandler (int sig);

#endif //MYSHELL_JOBPOOL_H
