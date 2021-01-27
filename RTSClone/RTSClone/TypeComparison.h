#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include <bitset>
#include <vector>

template <class Type>
class TypeComparison : private NonCopyable, private NonMovable
{
public:
	TypeComparison(const std::vector<Type>& cubeTypes)
	{
		for (auto cubeType : cubeTypes)
		{
			bitset.set(static_cast<int>(cubeType));
		}
	}

	const std::bitset<static_cast<int>(Type::Max) + 1>& getBitSet() const
	{
		return bitset;
	}

	bool isMatch(Type cubeType) const
	{
		std::bitset<static_cast<int>(Type::Max) + 1> otherBitSet;
		otherBitSet.set(static_cast<int>(cubeType));

		return (bitset & otherBitSet).any();
	}

	bool isMatch(const TypeComparison& other) const
	{
		return (bitset & other.getBitSet()).any();
	}

private:
	std::bitset<static_cast<int>(Type::Max) + 1> bitset{ 0 };
};