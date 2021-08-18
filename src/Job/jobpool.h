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
		kForeground,		// 作业在前台运行
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
	/**
	 * 新加入一个作业
	 * @param pid 作业的进程组 id
	 * @param str 作业的描述，即作业对应的命令
	 * @return 作业的编号
	 */
	int AddJob(pid_t pid, const std::string& str);

	/**
	 * 通过作业编号
	 * @param idx 作业的编号
	 * @return 作业的进程组 id
	 */
	int GetPID(int idx);

	/**
	 * 改变作业的状态
	 * @param pgid 作业的进程组 id
	 * @param status 作业改变后的状态
	 */
	void ChangeState(pid_t pgid, Job::Status status);	// 改变作业的状态

	/**
	 * 打印目前还在运行或者被挂起的作业
	 */
	void PrintJobs();

	/**
	 * 打印自从上次调用此函数之后所有状态发生改变的作业
	 */
	void PrintFinished();

	/**
	 * 子进程作业状态发生变化之后的回调函数，设计成友元函数
	 */
	friend void CHLDHandler (int sig);

private:
	std::vector<Job> _jobs;			// 作业列表
	std::queue<Job> _changed_jobs;	// 保存发生状态改变但还没有被打印过的作业
	static JobPool* thePool;		// 全局唯一作业池对象
};

/**
 * SIGCHLD 信号的处理函数
 * @param sig 信号
 */
void CHLDHandler (int sig);

/**
 * 向作业池汇报进程组状态变化
 * @param pid 进程组 id
 * @param status 此参数为调用 waitpid(pid, &status, XXX) 中 status 的值
 * @note
 * 	作业池中既记录了后台的进程，也记录了前台的进程，原因是前台进程可以通过键入 C-z
 * 	来转化为后台进程，因此我们必须追踪前台进程的状态。同时，CHLDHandler 不能通过
 * 	waitpid 来获得前台进程的状态，否则前台程序中的 waitpid 就可能失效 —— 同一个
 * 	进程应当只有一处在进行等待，所以对于前台进程，我们需要在 shell 等到其结束的时
 * 	候调用 ReportStatus 函数来显式地告知作业池。
 */
void ReportStatus(pid_t pid, int status);

#endif //MYSHELL_JOBPOOL_H
