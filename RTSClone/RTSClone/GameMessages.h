#pragma once

#include "GameMessageType.h"
#include "AABB.h"

//Caller is not meant to go out of scope. 
namespace GameMessages
{
	//CRTP - C++
	template <eGameMessageType T>
	struct BaseMessage
	{
		static eGameMessageType getType() { return T; };
	};

	template <eGameMessageType eventType>
	struct MapModification : public BaseMessage<eventType>
	{
		MapModification(const AABB& entityAABB)
			: entityAABB(entityAABB)
		{}

		const AABB& entityAABB;
	};

	struct UIDisplayResourceCount : public BaseMessage<eGameMessageType::UIDisplayResourceCount>
	{
		UIDisplayResourceCount(int resourceAmount)
			: resourceAmount(resourceAmount)
		{}

		const int resourceAmount;
	};
}