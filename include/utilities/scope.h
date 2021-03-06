#pragma once

#include <string>
#include <vector>
using VecStr = std::vector<std::string>;

struct proc;
struct AnimThreadInfo;

namespace nemesis
{
	struct scope
	{
		using funcptr = void (proc::*)(scope, VecStr&, AnimThreadInfo&) const;

		size_t front;
		size_t back;
		size_t size;
		VecStr olddata;
		std::vector<int> olddataint;
		funcptr func;

		scope() {}
		scope(size_t n_front, size_t n_back, funcptr n_func);
		scope(size_t n_front, size_t n_back, VecStr n_olddata, funcptr n_func);
		scope(size_t n_front, size_t n_back, std::vector<int> n_olddataint, funcptr n_func);
		scope(size_t n_front, size_t n_back, std::vector<int> n_olddataint, VecStr n_olddata, funcptr n_func);
	};
}
