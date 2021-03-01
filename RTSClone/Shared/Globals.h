#pragma once

#ifdef GAME
#include "EntityType.h"
#include "TypeComparison.h"
#endif // GAME
#include "glm/glm.hpp"
#include "AABB.h"
#include "FactionController.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <random>
#include <string>
#include <vector>

namespace Globals
{ 
	inline const std::string SHARED_FILE_DIRECTORY = "../Shared/";
	inline const std::string TEXT_HEADER_BEGINNING = "#";
	inline const std::string TEXT_HEADER_SCENERY = TEXT_HEADER_BEGINNING + "Scenery";
	inline const std::string TEXT_HEADER_MAP_SIZE = TEXT_HEADER_BEGINNING + "Map Size";
	inline const std::string TEXT_HEADER_FACTION_STARTING_RESOURCE = TEXT_HEADER_BEGINNING + "Faction Starting Resources";
	inline const std::string TEXT_HEADER_FACTION_STARTING_POPULATION = TEXT_HEADER_BEGINNING + "Faction Starting Population";
	inline const std::string TEXT_HEADER_FACTION_COUNT = TEXT_HEADER_BEGINNING + "Faction Count";
	inline const std::string TEXT_HEADER_MAIN_BASE_QUANTITY = TEXT_HEADER_BEGINNING + "Main Base Quantity";
	const std::array<std::string, static_cast<size_t>(eFactionController::Max) + 1> TEXT_HEADER_MAIN_BASES
	{
		TEXT_HEADER_BEGINNING + "Main Base Location_1",
		TEXT_HEADER_BEGINNING + "Main Base Location_2",
		TEXT_HEADER_BEGINNING + "Main Base Location_3",
		TEXT_HEADER_BEGINNING + "Main Base Location_4"
	};
	const std::array<std::string, static_cast<size_t>(eFactionController::Max) + 1> TEXT_HEADER_MAIN_BASE_MINERALS
	{
		TEXT_HEADER_BEGINNING + "Main Base Minerals_1",
		TEXT_HEADER_BEGINNING + "Main Base Minerals_2",
		TEXT_HEADER_BEGINNING + "Main Base Minerals_3",
		TEXT_HEADER_BEGINNING + "Main Base Minerals_4"
	};

	const glm::vec3 TERRAIN_POSITION = { 0.0f, -0.01f, 0.0f };
	const size_t MAX_LEVELS = 5;
	const unsigned int INVALID_OPENGL_ID = 0;
	const float GROUND_HEIGHT = 0.0f;
	const int MAP_SIZE = 30;
	const int NODE_SIZE = 6;

#ifdef GAME
	const int INVALID_ENTITY_ID = -1;
#endif // GAME
#ifdef LEVEL_EDITOR
	const int INVALID_GAMEOBJECT_ID = -1;
#endif // LEVEL_EDITOR

	const size_t MAX_MINERALS = 5;
	const glm::vec3 PLAYER_MATERIAL_DIFFUSE = { 1.0f, 0.2f, 0.2f };
	const glm::vec3 AI_1_MATERIAL_DIFFUSE = { 0.2f, 0.2f, 1.0f };
	const glm::vec3 AI_2_MATERIAL_DIFFUSE = { 1.0f, 1.0f, 0.2f };
	const glm::vec3 AI_3_MATERIAL_DIFFUSE = { 0.2f, 1.0f, 0.2f };
	inline std::array<glm::vec3, static_cast<size_t>(eFactionController::Max) + 1> FACTION_COLORS
	{
		PLAYER_MATERIAL_DIFFUSE,
		AI_1_MATERIAL_DIFFUSE,
		AI_2_MATERIAL_DIFFUSE,
		AI_3_MATERIAL_DIFFUSE
	};

	inline const std::string FACTION_MATERIAL_NAME_ID = "metal";
	const glm::uvec2 WINDOW_SIZE(1600, 900);

	const int HQ_STARTING_HEALTH = 25;
	const int BARRACKS_STARTING_HEALTH = 10;
	const int SUPPLY_DEPOT_STARTING_HEALTH = 10;
	const int UNIT_STARTING_HEALTH = 5;
	const int WORKER_STARTING_HEALTH = 2;
	const int LABORATORY_STARTING_HEALTH = 10;

	const int STARTING_RESOURCES = 1500;
	const int STARTING_POPULATION = 50;
	const int WORKER_RESOURCE_COST = 50;
	const int SUPPLY_DEPOT_RESOURCE_COST = 50;
	const int BARRACKS_RESOURCE_COST = 50;
	const int UNIT_RESOURCE_COST = 100;
	const int WORKER_POPULATION_COST = 1;
	const int UNIT_POPULATION_COST = 2;
	const int TURRET_RESOURCE_COST = 50;
	const int POPULATION_INCREMENT = 5;
	const int LABORATORY_RESOURCE_COST = 100;

	const float UNIT_STAT_BAR_WIDTH = 75.0f;
	const float WORKER_STAT_BAR_WIDTH = 60.0f;
	const float HQ_STAT_BAR_WIDTH = 150.0f;
	const float SUPPLY_DEPOT_STAT_BAR_WIDTH = 100.0f;
	const float BARRACKS_STAT_BAR_WIDTH = 100.0f;
	const float TURRET_STAT_BAR_WIDTH = 100.0f;
	const float LABORATORY_STAT_BAR_WIDTH = 150.0f;

	const int HQ_RESOURCES_COST = 100;
	const int HQ_POPULATION_COST = 0;
	const int SUPPLY_DEPOT_POPULATION_COST = 0;
	const int BARRACKS_POPULATION_COST = 0;
	const int TURRET_POPULATION_COST = 0;
	const int LABORATORY_POPULATION_COST = 0;

#ifdef GAME
	inline const std::array<float, static_cast<size_t>(eEntityType::Max) + 1> ENTITIES_STAT_BAR_WIDTH
	{
		UNIT_STAT_BAR_WIDTH,
		WORKER_STAT_BAR_WIDTH,
		HQ_STAT_BAR_WIDTH,
		SUPPLY_DEPOT_STAT_BAR_WIDTH,
		BARRACKS_STAT_BAR_WIDTH,
		TURRET_STAT_BAR_WIDTH,
		LABORATORY_STAT_BAR_WIDTH
	};

	inline const std::array<int, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_RESOURCE_COSTS
	{
		UNIT_RESOURCE_COST,
		WORKER_RESOURCE_COST,
		HQ_RESOURCES_COST,
		SUPPLY_DEPOT_RESOURCE_COST,
		BARRACKS_RESOURCE_COST,
		TURRET_RESOURCE_COST,
		LABORATORY_RESOURCE_COST
	};

	inline const std::array<int, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_POPULATION_COSTS
	{
		UNIT_POPULATION_COST,
		WORKER_POPULATION_COST,
		HQ_POPULATION_COST,
		SUPPLY_DEPOT_POPULATION_COST,
		BARRACKS_POPULATION_COST,
		TURRET_POPULATION_COST,
		LABORATORY_POPULATION_COST
	};

	const float UNIT_GRID_ATTACK_RANGE = 5.0f;
	const float UNIT_ATTACK_RANGE = UNIT_GRID_ATTACK_RANGE * static_cast<float>(Globals::NODE_SIZE);
#endif // GAME

	const int MAX_FACTION_SHIELD_AMOUNT = 5;
	const int FACTION_SHIELD_INCREASE_COST = 100;

	const glm::vec3 PROGRESS_BAR_COLOR = { 1.0f, 1.0f, 0.4f };
	const glm::vec3 HEALTH_BAR_COLOR = { 0.0f, 0.8f, 0.0f };
	const glm::vec3 SHIELD_BAR_COLOR = { 0.0f, 1.0f, 1.0f };
	const glm::vec3 BACKGROUND_BAR_COLOR = { 0.0f, 0.0f, 0.0f };
	const float DEFAULT_PROGRESS_BAR_HEIGHT = 5.0f;

#ifdef GAME
	inline const TypeComparison<eEntityType> BUILDING_TYPES({ eEntityType::Headquarters, eEntityType::Barracks, 
		eEntityType::SupplyDepot, eEntityType::Turret });
	inline const TypeComparison<eEntityType> UNIT_TYPES({ eEntityType::Unit, eEntityType::Worker });
	inline const TypeComparison<eEntityType> ATTACKING_ENTITY_TYPES({ eEntityType::Unit, eEntityType::Turret });
#endif // GAME

	inline void print(const std::string& text)
	{
		static sf::Clock clock;

		if (clock.getElapsedTime().asSeconds() > 0.5f)
		{
			std::cout << text << "\n\n";
			clock.restart();
		}
	}

	inline void printImmediately(const glm::vec3& position)
	{
		std::cout << position.x << "\n";
		std::cout << position.y << "\n";
		std::cout << position.z << "\n\n";
	}

	inline void print(const glm::vec3& position)
	{
		static sf::Clock clock;
		
		if (clock.getElapsedTime().asSeconds() > 1.0f)
		{
			std::cout << position.x << "\n";
			std::cout << position.y << "\n";
			std::cout << position.z << "\n\n";
			clock.restart();
		}
	}
	
	inline bool isWithinMapBounds(const AABB& AABB, const glm::ivec2& mapSize)
	{
		return AABB.getLeft() >= 0 &&
			AABB.getRight() < mapSize.x * Globals::NODE_SIZE &&
			AABB.getBack() >= 0 &&
			AABB.getForward() < mapSize.y * Globals::NODE_SIZE;
	}

	inline float getSqrDistance(const glm::vec2& positionB, const glm::vec2& positionA)
	{
		float x = glm::pow<float, float>(positionB.x - positionA.x, 2.0f);
		float y = glm::pow<float, float>(positionB.y - positionA.y, 2.0f);

		return x + y;
	}

	inline float getSqrDistance(const glm::vec3& positionB, const glm::vec3& positionA)
	{
		float x = glm::pow<float, float>(positionB.x - positionA.x, 2.0f);
		float y = glm::pow<float, float>(positionB.y - positionA.y, 2.0f);
		float z = glm::pow<float, float>(positionB.z - positionA.z, 2.0f);

		return x + y + z;
	}

	inline float getSqrDistance(glm::ivec2 positionB, glm::ivec2 positionA)
	{
		float x = glm::pow<float, float>(static_cast<float>(positionB.x - positionA.x), 2.0f);
		float y = glm::pow<float, float>(static_cast<float>(positionB.y - positionA.y), 2.0f);

		return x + y;
	}

	inline bool isOnNodePosition(const glm::vec3& position)
	{
		return static_cast<int>(position.x) % Globals::NODE_SIZE == 0 &&
			position.y == Globals::GROUND_HEIGHT &&
			static_cast<int>(position.z) % Globals::NODE_SIZE == 0;
	}

	inline bool isOnMiddlePosition(const glm::vec3& position)
	{
		return static_cast<int>(position.x) % Globals::NODE_SIZE == Globals::NODE_SIZE / 2 &&
			position.y == Globals::GROUND_HEIGHT &&
			static_cast<int>(position.z) % Globals::NODE_SIZE == Globals::NODE_SIZE / 2;
	}

	inline glm::vec3 convertToNodePosition(const glm::vec3& position)
	{
		return position - glm::mod(position, static_cast<float>(Globals::NODE_SIZE));
	}

	inline glm::vec3 convertToMiddleGridPosition(const glm::vec3& position)
	{
		assert(static_cast<int>(position.x) % Globals::NODE_SIZE == 0 &&
			position.y == Globals::GROUND_HEIGHT &&
			static_cast<int>(position.z) % Globals::NODE_SIZE == 0);

		return { position.x + static_cast<float>(Globals::NODE_SIZE) / 2.0f, Globals::GROUND_HEIGHT, 
			position.z + static_cast<float>(Globals::NODE_SIZE) / 2.0f };
	}

	inline glm::vec3 convertToWorldPosition(const glm::ivec2& position)
	{
		return Globals::convertToMiddleGridPosition(
			{ position.x * Globals::NODE_SIZE, Globals::GROUND_HEIGHT, position.y * Globals::NODE_SIZE });
	}

	inline glm::ivec2 convertToGridPosition(const glm::vec3& position)
	{
		return { position.x / Globals::NODE_SIZE, position.z / Globals::NODE_SIZE };
	}

	inline int convertTo1D(const glm::ivec2& position, const glm::ivec2& mapSize)
	{
		return position.x * mapSize.x + position.y;
		//return position.x * Globals::MAP_SIZE + position.y;
	}

	inline glm::vec3 moveTowards(const glm::vec3& currentPosition, const glm::vec3& targetPosition, float maxDistanceDelta)
	{
		float magnitude = glm::distance(targetPosition, currentPosition);
		if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
		{
			return targetPosition;
		}

		return currentPosition + glm::vec3(targetPosition - currentPosition) / magnitude * maxDistanceDelta;
	}

	inline int getRandomNumber(int min, int max)
	{
		static std::random_device rd;  //Will be used to obtain a seed for the random number engine
		static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<> dis(min, max);

		return dis(gen);
	}

	inline float getRandomNumber(float min, float max)
	{
		static std::random_device rd;  //Will be used to obtain a seed for the random number engine
		static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> distrib(min, max);

		return distrib(gen);
	}

	inline float getAngle(const glm::vec3& positionB, const glm::vec3& positionA, float offsetYRotation = 90.0f)
	{
		return glm::degrees(atan2(positionB.z - positionA.z, positionA.x - positionB.x)) + offsetYRotation;
	}

	inline const glm::vec3& getNextPathDestination(const std::vector<glm::vec3>& pathToPosition, const glm::vec3& position)
	{
		if (!pathToPosition.empty())
		{
			return pathToPosition.front();
		}

		return position;
	}
}