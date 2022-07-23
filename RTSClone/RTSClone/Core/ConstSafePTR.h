#pragma once

template <typename T>
class ConstSafePTR
{
public:
	ConstSafePTR(T* data)
		: data(data)
	{}
	ConstSafePTR(const ConstSafePTR&) = delete;
	ConstSafePTR& operator=(const ConstSafePTR&) = delete;
	ConstSafePTR(ConstSafePTR&& rhs) noexcept
	{
		std::swap(data, rhs.data);
	}
	ConstSafePTR& operator=(ConstSafePTR&& rhs) noexcept
	{
		std::swap(data, rhs.data);
		return *this;
	}

	const T* operator->() const { return data; }
	T* operator->() { return data; }

	const T* operator*() const { return data; }
	T* operator*() { return data; }

	bool operator!() { return !data; }

private:
	T* data;
};