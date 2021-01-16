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
		AddToMap(const AABB& AABB)
			: AABB(AABB) {}

		const AABB& AABB;
	};

	struct RemoveFromMap : public BaseMessage<eGameMessageType::RemoveEntityFromMap>
	{
		RemoveFromMap(const AABB& AABB)
			: AABB(AABB) {}

		const AABB& AABB;
	};

	struct NewMapSize : public BaseMessage<eGameMessageType::NewMapSize>
	{
		NewMapSize(glm::ivec2 mapSize)
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
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health,
			int shield);
		UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, int health,
			int shield, int queueSize);

		eFactionController owningFaction;
		int entityID;
		eEntityType entityType;
		int health;
		int shield;
		int queueSize;
	};

	struct UIDisplayWinner : public BaseMessage<eGameMessageType::UIDisplayWinner>
	{
		UIDisplayWinner();
		UIDisplayWinner(eFactionController winningFaction);

		eFactionController winningFaction;
	};
}