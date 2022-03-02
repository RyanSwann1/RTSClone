#pragma once

#include <atomic>

namespace id_generator
{
	inline int new_id()
	{
		static std::atomic<int> id = 0;
		return id++;
	}
}