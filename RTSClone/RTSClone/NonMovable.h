#pragma once

struct NonMovable
{
	NonMovable() {}
	NonMovable(NonMovable&&) = delete;
	NonMovable& operator=(NonMovable&&) = delete;
};