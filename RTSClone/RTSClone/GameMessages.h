#pragma once

#include "GameMessageType.h"
#include "AABB.h"
#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

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
		UIDisplayPlayerDetails()
			: resourceAmount(0),
			currentPopulationAmount(0),
			maximumPopulationAmount(0)
		{}

		UIDisplayPlayerDetails(int resourceAmount, int currentPopulationAmount, int maximumPopulationAmount)
			: resourceAmount(resourceAmount),
			currentPopulationAmount(currentPopulationAmount),
			maximumPopulationAmount(maximumPopulationAmount)
		{}

		int resourceAmount;
		int currentPopulationAmount;
		int maximumPopulationAmount;
	};

	struct UIDisplayEntity : public BaseMessage<eGameMessageType::UIDisplayEntity>
	{
		UIDisplayEntity()
			: owningFaction(),
			entityID(Globals::INVALID_ENTITY_ID),
			entityType(),
			health(0),
			queueSize(0)
		{}
		UIDisplayEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health)
			: owningFaction(owningFaction),
			entityID(entityID),
			entityType(entityType),
			health(health),
			queueSize(0)
		{
			assert(!Globals::UNIT_SPAWNER_TYPES.isMatch(entityType));
		}
		UIDisplayEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health, int queueSize)
			: owningFaction(owningFaction),
			entityID(entityID),
			entityType(entityType),
			health(health),
			queueSize(queueSize)
		{
			assert(Globals::UNIT_SPAWNER_TYPES.isMatch(entityType));
		}

		eFactionController owningFaction;
		int entityID;
		eEntityType entityType;
		int health;
		int queueSize;
	};

	struct UIDisplayWinner : public BaseMessage<eGameMessageType::UIDisplayWinner>
	{
		UIDisplayWinner(eFactionController winningFaction)
			: winningFaction(winningFaction)
		{}

		eFactionController winningFaction;
	};
}