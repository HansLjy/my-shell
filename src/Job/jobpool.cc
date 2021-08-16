//
// Created by hansljy on 2021/8/15.
//

#include "jobpool.h"

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

void JobPool::FinishJob(int idx) {
	_finished_jobs.push(_jobs[idx]);	// 插入已经完成的作业中
	_jobs[idx]._status = Job::kDone;	// Lazy 删除
}

void JobPool::SuspendJob(int idx) {
	_jobs[idx]._status = Job::kSuspended;	// 挂起作业
}

void JobPool::PrintJobs() {
	int size = _jobs.size();
	for (int i = 0; i < size; i++) {
		const auto& job = _jobs[i];
		if (job._status != Job::kDone) {
			fprintf(stdout, "[%d]\t%d\t\t%s\n", i, job._pid, job._str.c_str());
		}
	}
}

void JobPool::PrintFinished() {
	fprintf(stdout, "PID\t\tCOMMAND");
	while (!_finished_jobs.empty()) {
		Job job = _finished_jobs.front();
		fprintf(stdout, "%d\t\t%s\n", job._pid, job._str.c_str());
		_finished_jobs.pop();
	}
}