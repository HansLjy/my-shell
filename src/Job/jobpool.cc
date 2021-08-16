//
// Created by hansljy on 2021/8/15.
//

#include "jobpool.h"
#include <sys/wait.h>

Job::Job(Status status, pid_t pid, const std::string& str)
	: _status(status), _pid(pid), _str(str) {}

JobPool* JobPool::thePool = nullptr;

JobPool *JobPool::Instance() {
	if (thePool == nullptr) {
		thePool = new JobPool;
	}
	return thePool;
}

int JobPool::AddJob(pid_t pid, const std::string &str) {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		if (_jobs[i]._status == Job::kDone) {
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
	if (idx >= _jobs.size()) {
		return -1;
	}
	if (_jobs[idx]._status == Job::kDone) {
		return -1;
	}
	return _jobs[idx]._pid;
}

void JobPool::ChangeState(pid_t pgid, Job::Status status) {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		if (_jobs[i]._pid == pgid) {
			_jobs[i]._status = status;
			if (status == Job::kDone || status == Job::kKilled) {
				_finished_jobs.push(_jobs[i]);
			}
			break;
		}
	}
}

void JobPool::PrintJobs() {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		const auto& job = _jobs[i];
		if (job._status != Job::kDone && job._status != Job::kKilled) {
			fprintf(stdout, "[%d]", i);
			switch (job._status) {
				case Job::kRunning:
					fprintf(stdout, "\tRunning\t");
					break;
				case Job::kSuspended:
					fprintf(stdout, "\tSuspended\t");
					break;
			}
			fprintf(stdout, "%s\n", job._str.c_str());
		}
	}
}

void JobPool::PrintFinished() {
	while (!_finished_jobs.empty()) {
		Job job = _finished_jobs.front();
		switch (job._status) {
			case Job::kDone:
				fprintf(stdout, "Done:\t");
				break;
			case Job::kKilled:
				fprintf(stdout, "Killed:\t");
				break;
		}
		fprintf(stdout, "%d\t\t%s\n", job._pid, job._str.c_str());
		_finished_jobs.pop();
	}
}

// 为什么要费这么大劲枚举呢……这是个悲伤的故事
void CHLDHandler (int sig) {
	int status;
	auto job_pool = JobPool::Instance();
	auto & jobs = job_pool->_jobs;
	for (auto & job : jobs) {
		if (job._status == Job::kDone || job._status == Job::kKilled)
			continue;
		if(waitpid(job._pid, &status, WNOHANG) > 0) {
			auto pid = job._pid;
			if (WIFEXITED(status)) {
				// 正常退出
				job_pool->ChangeState(pid, Job::kDone);
				fprintf(stderr, "Done\n");
			} else if (WIFSTOPPED(status)) {
				// 挂起
				job_pool->ChangeState(pid, Job::kSuspended);
				fprintf(stderr, "Suspended\n");
			} else if (WIFCONTINUED(status)) {
				// 继续
				job_pool->ChangeState(pid, Job::kRunning);
				fprintf(stderr, "Continued\n");
			} else {
				// 因为各种原因被杀死了
				job_pool->ChangeState(pid, Job::kKilled);
				fprintf(stderr, "Killed\n");
			}
			break;
		}
	}
}

void STGHandler (int sig) {
	printf("Ctrl-Z detected\n");
}
// 状态改变的子进程的 id
