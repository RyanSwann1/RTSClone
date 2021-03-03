#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

class Entity;
class Mineral;
class SceneryGameObject;
class AABB;
//Caller is not meant to go out of scope. 
namespace GameMessages
{
	struct UIClearDisplaySelectedEntity {};
	struct UIClearWinner {};

	struct AddAABBToMap
	{
		AddAABBToMap(const AABB & aabb)
			: aabb(aabb) {}
		const AABB& aabb;
	};

	struct RemoveAABBFromMap
	{
		RemoveAABBFromMap(const AABB& aabb)
			: aabb(aabb) {}
		const AABB& aabb;
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