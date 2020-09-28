#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include <assert.h>

class PathFinding;
class PathFindingLocator : private NonCopyable, private NonMovable
{
public:
	static void provide(PathFinding& instance)
	{
		assert(!m_instance);
		m_instance = &instance;
	}

	static PathFinding& get()
	{
		assert(m_instance);
		return *m_instance;
	}

private:
	PathFindingLocator();
	static PathFinding* m_instance;
};