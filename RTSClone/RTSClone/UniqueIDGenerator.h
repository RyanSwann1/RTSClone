#pragma once

class UniqueIDGenerator 
{
public:
	UniqueIDGenerator(const UniqueIDGenerator&) = delete;
	UniqueIDGenerator& operator=(const UniqueIDGenerator&) = delete;
	UniqueIDGenerator(UniqueIDGenerator&&) = delete;
	UniqueIDGenerator&& operator=(UniqueIDGenerator&&) = delete;

	static UniqueIDGenerator& getInstance()
	{
		static UniqueIDGenerator instance;
		return instance;
	}

	int getUniqueID()
	{
		return m_uniqueEntityIDCounter++;
	}

private:
	UniqueIDGenerator()
		: m_uniqueEntityIDCounter(0)
	{}

	int m_uniqueEntityIDCounter;
};