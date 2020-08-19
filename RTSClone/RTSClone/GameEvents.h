#pragma once

#include "GameEventType.h"
#include "AABB.h"

namespace GameEvents
{
	//CRTP - C++
	template <eGameEventType T>
	struct BaseEvent
	{
		static eGameEventType getType() { return T; };
	};

	template <eGameEventType eventType>
	struct MapModification : public BaseEvent<eventType>
	{
		MapModification(const AABB& entityAABB)
			: entityAABB(entityAABB)
		{}

		const AABB& entityAABB;
	};
}