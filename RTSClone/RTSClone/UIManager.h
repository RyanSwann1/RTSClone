#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"

namespace GameMessages
{
	struct UIDisplayPlayerDetails;
};
class UIManager : private NonCopyable, private NonMovable
{
public:
	UIManager();
	~UIManager();

private:
	void onDisplayResourceCount(const GameMessages::UIDisplayPlayerDetails& gameMessage);
};