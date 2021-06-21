#pragma once

#include "Globals.h"
#include "EntityType.h"
#include "AIAction.h"
#include <array>
#include <vector>

namespace AI
{
	constexpr float DELAY_TIMER_EXPIRATION = 5.0f;
	constexpr float IDLE_TIMER_EXPIRATION = 1.0f;
	constexpr float MIN_SPAWN_TIMER_EXPIRATION = 7.5f;
	constexpr float MAX_SPAWN_TIMER_EXPIRATION = 15.0f;
	constexpr float MAX_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 18.0f;
	constexpr float MIN_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 5.0f;
	constexpr float DISTANCE_FROM_MINERALS = static_cast<float>(Globals::NODE_SIZE) * 7.0f;
	constexpr float MIN_BASE_EXPANSION_TIME = 2.0f;
	constexpr float MAX_BASE_EXPANSION_TIME = MIN_BASE_EXPANSION_TIME * 2.0f;
	
	constexpr int MIN_WORKERS_AT_BASE = 2;

	//Defensive
	constexpr size_t DEFENSIVE_MAX_TURRETS = 5;

	enum class eBehaviour
	{
		Defensive,
		Aggressive,
		Max = Aggressive
	};

	inline const std::array<int, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_MODIFIERS_DEFENSIVE
	{
		1, //Unit
		4, //Worker
		1, //Headquarters
		1, //SupplyDepot
		2, //Barracks
		3, //Turret
		1  //Laboratory
	};

	inline const std::array<int, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_MODIFIERS_AGGRESSIVE
	{
		1, //Unit
		4, //Worker
		1, //Headquarters
		1, //SupplyDepot
		2, //Barracks
		3, //Turret
		1  //Laboratory
	};

	inline int getEntityModifier(eEntityType entityType, eBehaviour behaviour)
	{
		if (behaviour == eBehaviour::Defensive) { return ENTITY_MODIFIERS_DEFENSIVE[static_cast<int>(behaviour)]; }
		else { return ENTITY_MODIFIERS_AGGRESSIVE[static_cast<int>(behaviour)]; }
	}

	constexpr int MAX_LABORATORY_ALL = 1;
	constexpr int MAX_SUPPLY_DEPOT_DEFENSIVE = 2;
	constexpr int MAX_BARRACKS_DEFENSIVE = 2;
	constexpr int MAX_TURRETS_DEFENSIVE = 4;

	inline const std::array<std::vector<eAIActionType>, static_cast<size_t>(eBehaviour::Max) + 1> STARTING_BUILD_ORDERS
	{
		//Defensive
		std::vector<eAIActionType> {
			eAIActionType::SpawnWorker,
			eAIActionType::SpawnWorker,
			eAIActionType::BuildLaboratory,
			eAIActionType::BuildSupplyDepot,
			eAIActionType::BuildSupplyDepot,
			eAIActionType::BuildTurret,
			eAIActionType::SpawnWorker,
		},

		//Aggressive
		std::vector<eAIActionType> {
			eAIActionType::SpawnWorker,
			eAIActionType::SpawnWorker,
			eAIActionType::BuildLaboratory,
			eAIActionType::BuildBarracks,
			eAIActionType::BuildTurret,
			eAIActionType::BuildSupplyDepot,
			eAIActionType::SpawnWorker,
		},
	};
}