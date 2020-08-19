#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"

class UniqueEntityIDDistributer : private NonCopyable, private NonMovable
{
public:
	static UniqueEntityIDDistributer& getInstance()
	{
		static UniqueEntityIDDistributer instance;
		return instance;
	}

	int getUniqueEntityID()
	{
		int ID = m_uniqueEntityIDCounter;
		++m_uniqueEntityIDCounter;
		return ID;
	}

private:
	UniqueEntityIDDistributer()
		: m_uniqueEntityIDCounter(0)
	{}

	int m_uniqueEntityIDCounter;
};