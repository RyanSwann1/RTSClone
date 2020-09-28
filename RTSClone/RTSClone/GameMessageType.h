#pragma once

enum class eGameMessageType
{
	AddEntityToMap = 0,
	RemoveEntityFromMap,
	NewMapSize,
	UIDisplayPlayerDetails,
	UIDisplaySelectedEntity,
	UIClearDisplaySelectedEntity,
	UIDisplayWinner,
	UIClearWinner,
	Max = UIClearWinner
};