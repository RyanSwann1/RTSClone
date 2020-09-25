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