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

	struct UIDisplaySelectedEntity : public BaseMessage<eGameMessageType::UIDisplaySelectedEntity>
	{
		UIDisplaySelectedEntity()
			: owningFaction(),
			entityID(Globals::INVALID_ENTITY_ID),
			entityType(),
			health(0),
			queueSize(0),
			spawnTime(0.0f)
		{}
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health)
			: owningFaction(owningFaction),
			entityID(entityID),
			entityType(entityType),
			health(health),
			queueSize(0),
			spawnTime(0.0f)
		{
			assert(!Globals::UNIT_SPAWNER_TYPES.isMatch(entityType));
		}
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health, 
			int queueSize, float spawnTime)
			: owningFaction(owningFaction),
			entityID(entityID),
			entityType(entityType),
			health(health),
			queueSize(queueSize),
			spawnTime(spawnTime)
		{
			assert(Globals::UNIT_SPAWNER_TYPES.isMatch(entityType));
		}

		eFactionController owningFaction;
		int entityID;
		eEntityType entityType;
		int health;
		int queueSize;
		float spawnTime;
	};

	struct UIDisplayWinner : public BaseMessage<eGameMessageType::UIDisplayWinner>
	{
		UIDisplayWinner()
			: winningFaction()
		{}
		UIDisplayWinner(eFactionController winningFaction)
			: winningFaction(winningFaction)
		{}

		eFactionController winningFaction;
	};
}