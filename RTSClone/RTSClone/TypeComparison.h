#pragma once

#include <bitset>
#include <array>

template <class Type, size_t Size>
class TypeComparison 
{
public:
	constexpr TypeComparison(std::array<Type, Size> types)
	{
		for (auto type : types)
		{
			bitset.set(static_cast<int>(type));
		}
	}

	std::bitset<static_cast<int>(Type::Max) + 1> getBitSet() const
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
	std::bitset<static_cast<int>(Type::Max) + 1> bitset = { 0 };
};