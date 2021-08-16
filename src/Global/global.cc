//
// Created by hansljy on 2021/8/15.
//

#include "global.h"

SpecialVarPool* SpecialVarPool::thePool = nullptr;

SpecialVarPool *SpecialVarPool::Instance() {
	if (thePool == nullptr) {
		thePool = new SpecialVarPool;
	}
	return thePool;
}

void SpecialVarPool::SetArg(int idx, const std::string &val) {
	if (idx >= _args.size()) {
		_args.resize(idx + 1);
	}
	_args[idx] = val;
}

std::string SpecialVarPool::GetArg(int idx) {
	if (idx >= _args.size() || idx < 0) {
		return "";				// 返回空串
	}
	return _args[idx];
}

int SpecialVarPool::GetArgc() {
	return _args.size() - 1;	// [0, argc]
}

void SpecialVarPool::SetReturn(int ret) {
	_ret = ret;
}

int SpecialVarPool::GetReturn() {
	return _ret;
}

bool SpecialVarPool::Shift(int n) {
	int argc = _args.size();
	if (n < 0 || n >= argc - 1) {
		return false;
	}
	for (int i = 1; i < argc - n; i++) {
		_args[i] = _args[i + n];	// 移动
	}
	for (int i = argc - n; i < argc; i++) {
		_args[i] = "";				// 清空多余的变量
	}
	return true;
}

void SpecialVarPool::Clear() {
	_args.clear();
}