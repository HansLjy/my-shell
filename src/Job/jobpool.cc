//
// Created by hansljy on 2021/8/15.
//

#include "jobpool.h"
#include <sys/wait.h>

Job::Job(Status status, pid_t pid, const std::string& str)
	: _status(status), _pid(pid), _str(str) {}

JobPool* JobPool::thePool = nullptr;

JobPool *JobPool::Instance() {
	// Singleton 模式
	if (thePool == nullptr)
		thePool = new JobPool;
	return thePool;
}

int JobPool::AddJob(pid_t pid, const std::string &str) {
	int size = _jobs.size();	// 数组大小
	for (int i = 0; i < size; i++) {
		if (_jobs[i]._status == Job::kDone || _jobs[i]._status == Job::kKilled) {
			// 如果任务已经完成了，则可以被替换掉
			_jobs[i] = Job(Job::kRunning, pid, str);
			return i;
		}
	}
	_jobs.push_back(Job(Job::kRunning, pid, str));	// 如果没有空位，就在最后插入
	return size;
}

// 返回 -1 表示遇到了不在运行的进程
int JobPool::GetPID(int idx) {
	if (idx >= _jobs.size() || idx < 0) {
		return -1;
	}
	if (_jobs[idx]._status == Job::kDone || _jobs[idx]._status == Job::kKilled) {
		// 作业不在运行
		return -1;
	}
	return _jobs[idx]._pid;
}

void JobPool::ChangeState(pid_t pgid, Job::Status status) {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		if (_jobs[i]._pid == pgid) {
			// 找到 pid
			_jobs[i]._status = status;
			_changed_jobs.push(_jobs[i]);
			break;
		}
	}
}

void JobPool::PrintJobs() {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		// 遍历作业
		const auto& job = _jobs[i];
		if (job._status != Job::kDone && job._status != Job::kKilled) {
			fprintf(stdout, "[%d]\t%d", i, job._pid);
			switch (job._status) {
				case Job::kRunning:	// 运行
					fprintf(stdout, "\tRunning\t");
					break;
				case Job::kSuspended:	// 挂起
					fprintf(stdout, "\tSuspended\t");
					break;
				case Job::kForeground:	// 前台
					fprintf(stdout, "\tForeground\t");
					break;
			}
			fprintf(stdout, "%s\n", job._str.c_str());
		}
	}
}

void JobPool::PrintFinished() {
	while (!_changed_jobs.empty()) {
		Job job = _changed_jobs.front();
		switch (job._status) {
			case Job::kDone:	// 完成
				fprintf(stdout, "Done:\t");
				break;
			case Job::kKilled:	// 杀死
				fprintf(stdout, "Killed:\t");
				break;
			case Job::kSuspended:	// 挂起
				fprintf(stdout, "Suspended:\t");
				break;
			case Job::kRunning:	// 运行
				fprintf(stdout, "Continued:\t");
				break;
		}
		fprintf(stdout, "%d\t\t%s\n", job._pid, job._str.c_str());
		_changed_jobs.pop();
	}
}

// 为什么要费这么大劲枚举呢……这是个悲伤的故事
void CHLDHandler (int sig) {
	int status;
	auto job_pool = JobPool::Instance();
	auto & jobs = job_pool->_jobs;	// 作业
	for (auto & job : jobs) {
		if (
			job._status == Job::kDone
			|| job._status == Job::kKilled
			|| job._status == Job::kForeground
		)
			continue;
		if(waitpid(job._pid, &status, WNOHANG | WUNTRACED | WCONTINUED) > 0) {
			// 找到状态发生变化的作业
			ReportStatus(job._pid, status);
			break;
		}
	}
}

void ReportStatus(pid_t pid, int status) {
	auto job_pool = JobPool::Instance();
	if (WIFEXITED(status)) {
		// 正常退出
		job_pool->ChangeState(pid, Job::kDone);
	} else if (WIFSTOPPED(status)) {
		// 挂起
		job_pool->ChangeState(pid, Job::kSuspended);
	} else if (WIFCONTINUED(status)) {
		// 继续
		job_pool->ChangeState(pid, Job::kRunning);
	} else {
		// 因为各种原因被杀死了
		job_pool->ChangeState(pid, Job::kKilled);
	}
}

