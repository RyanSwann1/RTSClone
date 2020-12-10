#pragma once

#include "GameMessageType.h"
#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

class AABB;
//Caller is not meant to go out of scope. 
namespace GameMessages
{
	//CRTP - C++
	template <eGameMessageType T>
	struct BaseMessage
	{
		static eGameMessageType getType() { return T; };
	};

	struct AddToMap : public BaseMessage<eGameMessageType::AddEntityToMap>
	{
		AddToMap(const AABB& AABB, int entityID)
			: AABB(AABB),
			entityID(entityID)
		{}

		const AABB& AABB;
		const int entityID;
	};

	struct RemoveFromMap : public BaseMessage<eGameMessageType::RemoveEntityFromMap>
	{
		RemoveFromMap(const AABB& AABB, int entityID)
			: AABB(AABB),
			entityID(entityID) {}

		const AABB& AABB;
		const int entityID;
	};

	struct NewMapSize : public BaseMessage<eGameMessageType::NewMapSize>
	{
		NewMapSize(const glm::ivec2& mapSize)
			: mapSize(mapSize)
		{}

		const glm::ivec2 mapSize;
	};

	struct UIDisplayPlayerDetails : public BaseMessage<eGameMessageType::UIDisplayPlayerDetails>
	{
		UIDisplayPlayerDetails();
		UIDisplayPlayerDetails(int resourceAmount, int currentPopulationAmount, int maximumPopulationAmount);

		int resourceAmount;
		int currentPopulationAmount;
		int maximumPopulationAmount;
	};

	struct UIDisplaySelectedEntity : public BaseMessage<eGameMessageType::UIDisplaySelectedEntity>
	{
		UIDisplaySelectedEntity();
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health);
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health, 
			float buildTime);
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health,
			int queueSize, float spawnTime);

		eFactionController owningFaction;
		int entityID;
		eEntityType entityType;
		int health;
		int queueSize;
		float spawnTime;
		float buildTime;
	};

	struct UIDisplayWinner : public BaseMessage<eGameMessageType::UIDisplayWinner>
	{
		UIDisplayWinner();
		UIDisplayWinner(eFactionController winningFaction);

		eFactionController winningFaction;
	};
}