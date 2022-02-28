#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "Globals.h"

class Entity;
class Mineral;
class AABB;
class Worker;
class Map;
//Caller is not meant to go out of scope. 
namespace GameMessages
{
	struct UIClearDisplaySelectedEntity {};
	struct UIClearSelectedMineral {};
	struct UIClearWinner {};

	struct AddAABBToMap
	{
		const AABB& aabb;
	};

	struct RemoveAABBFromMap
	{
		const AABB& aabb;
	};

	struct AddUnitPositionToMap
	{
		const glm::vec3& position;
		const int ID = Globals::INVALID_ENTITY_ID;
	};

	struct RemoveUnitPositionFromMap
	{
		const glm::vec3& position;
		const int ID = Globals::INVALID_ENTITY_ID;
	};

	struct NewMapSize 
	{
		const glm::ivec2 mapSize;
	};

	struct UIDisplayPlayerDetails 
	{
		int resourceAmount = 0;
		int currentPopulationAmount = 0;
		int maximumPopulationAmount = 0;
	};

	struct UIDisplaySelectedEntity 
	{
		eFactionController owningFaction = eFactionController::None;
		int entityID = Globals::INVALID_ENTITY_ID;
		eEntityType entityType;
		int health = 0;
		int shield = 0;
		int queueSize = 0;
	};

	struct UIDisplayWinner
	{
		eFactionController winningFaction = eFactionController::None;
	};

	struct UIDisplaySelectedMineral
	{
		const Mineral& mineral;
	};

	struct GetEntity
	{
		const int entityID = Globals::INVALID_ENTITY_ID;
	};

	struct GetClosestHeadquarters
	{
		const glm::vec3 position;
	};

	struct CreateBuilding
	{
		const Worker& worker;
		const Map& map;
	};
}