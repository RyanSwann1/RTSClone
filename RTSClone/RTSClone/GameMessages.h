#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

class Entity;
class Mineral;
//Caller is not meant to go out of scope. 
namespace GameMessages
{
	struct UIClearDisplaySelectedEntity {};
	struct UIClearWinner {};

	struct AddBuildingToMap 
	{
		AddBuildingToMap(const Entity& entity);
		const Entity& entity;
	};

	struct RemoveBuildingFromMap 
	{
		RemoveBuildingFromMap(const Entity& entity);
		const Entity& entity;
	};

	struct AddMineralToMap
	{
		AddMineralToMap(const Mineral& mineral);
		const Mineral& mineral;
	};

	struct RemoveMineralFromMap
	{
		RemoveMineralFromMap(const Mineral& mineral);
		const Mineral& mineral;
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