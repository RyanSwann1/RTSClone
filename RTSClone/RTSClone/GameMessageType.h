#pragma once

enum class eGameMessageType
{
	AddEntityToMap = 0,
	RemoveEntityFromMap,
	UIDisplayPlayerDetails,
	UIDisplayEntity,
	UIClearDisplayEntity,
	Max = UIClearDisplayEntity
};