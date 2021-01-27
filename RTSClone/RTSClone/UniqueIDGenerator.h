#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"

class UniqueIDGenerator : private NonCopyable, private NonMovable
{
public:
	static UniqueIDGenerator& getInstance()
	{
		static UniqueIDGenerator instance;
		return instance;
	}

	int getUniqueID()
	{
		int ID = m_uniqueEntityIDCounter;
		++m_uniqueEntityIDCounter;
		return ID;
	}

private:
	UniqueIDGenerator()
		: m_uniqueEntityIDCounter(0)
	{}

	int m_uniqueEntityIDCounter;
};