#pragma once

#include <atomic>

namespace id_generator
{
	inline int gen()
	{
		static std::atomic<int> id = 0;
		return id++;
	}
}