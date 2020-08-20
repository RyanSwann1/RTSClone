#pragma once

#include "GameMessageType.h"
#include "AABB.h"

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
}