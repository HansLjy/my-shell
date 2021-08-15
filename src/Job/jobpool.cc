//
// Created by hansljy on 2021/8/15.
//

#include "jobpool.h"

JobPool* JobPool::thePool = nullptr;

JobPool *JobPool::Instance() {
	if (thePool == nullptr) {
		thePool = new JobPool;
	}
	return thePool;
}