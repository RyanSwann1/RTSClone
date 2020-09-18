#pragma once

#include "GameMessageType.h"
#include "AABB.h"
#include "EntityType.h"
#include "FactionController.h"

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

	struct UIDisplayPlayerDetails : public BaseMessage<eGameMessageType::UIDisplayPlayerDetails>
	{
		UIDisplayPlayerDetails(int resourceAmount, int currentPopulationAmount, int maximumPopulationAmount)
			: resourceAmount(resourceAmount),
			currentPopulationAmount(currentPopulationAmount),
			maximumPopulationAmount(maximumPopulationAmount)
		{}

		const int resourceAmount;
		const int currentPopulationAmount;
		const int maximumPopulationAmount;
	};

	struct UIDisplayEntity : public BaseMessage<eGameMessageType::UIDisplayEntity>
	{
		UIDisplayEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int quantity)
			: owningFaction(owningFaction),
			entityID(entityID),
			entityType(entityType),
			quantity(quantity)
		{}

		const eFactionController owningFaction;
		const int entityID;
		const eEntityType entityType;
		const int quantity;
	};
}