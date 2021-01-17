#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

class AABB;
//Caller is not meant to go out of scope. 
namespace GameMessages
{
	struct UIClearDisplaySelectedEntity {};
	struct UIClearWinner {};

	struct AddToMap 
	{
		AddToMap(const AABB& AABB)
			: AABB(AABB) {}

		const AABB& AABB;
	};

	struct RemoveFromMap 
	{
		RemoveFromMap(const AABB& AABB)
			: AABB(AABB) {}

		const AABB& AABB;
	};

	struct NewMapSize 
	{
		NewMapSize(glm::ivec2 mapSize)
			: mapSize(mapSize)
		{}

		const glm::ivec2 mapSize;
	};

	struct UIDisplayPlayerDetails 
	{
		UIDisplayPlayerDetails();
		UIDisplayPlayerDetails(int resourceAmount, int currentPopulationAmount, int maximumPopulationAmount);

		int resourceAmount;
		int currentPopulationAmount;
		int maximumPopulationAmount;
	};

	struct UIDisplaySelectedEntity 
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

	struct UIDisplayWinner
	{
		UIDisplayWinner();
		UIDisplayWinner(eFactionController winningFaction);

		eFactionController winningFaction;
	};
}