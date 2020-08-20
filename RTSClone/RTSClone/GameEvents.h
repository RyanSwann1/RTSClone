#pragma once

#include "GameMessageType.h"
#include "AABB.h"

namespace GameEvents
{
	//CRTP - C++
	template <eGameMessageType T>
	struct BaseEvent
	{
		static eGameMessageType getType() { return T; };
	};

	template <eGameMessageType eventType>
	struct MapModification : public BaseEvent<eventType>
	{
		MapModification(const AABB& entityAABB)
			: entityAABB(entityAABB)
		{}

		const AABB& entityAABB;
	};
}