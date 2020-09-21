#pragma once

enum class eGameMessageType
{
	AddEntityToMap = 0,
	RemoveEntityFromMap,
	UIDisplayPlayerDetails,
	UIDisplaySelectedEntity,
	UIClearDisplaySelectedEntity,
	UIDisplayWinner,
	UIClearWinner,
	Max = UIClearWinner
};