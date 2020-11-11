#pragma once

#ifdef GAME
#include "EntityType.h"
#endif // GAME

#include "glm/glm.hpp"
#include "AABB.h"
#include "FactionController.h"
#include "TypeComparison.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <random>
#include <string>

namespace Globals
{ 
	inline const std::string SHARED_FILE_DIRECTORY = "../Shared/";
	inline const std::string TEXT_HEADER_BEGINNING = "#";
	inline const std::string TEXT_HEADER_SCENERY = TEXT_HEADER_BEGINNING + "Scenery";
	inline const std::string TEXT_HEADER_MAP_SIZE = TEXT_HEADER_BEGINNING + "Map Size";
	inline const std::string TEXT_HEADER_FACTION_STARTING_RESOURCE = TEXT_HEADER_BEGINNING + "Faction Starting Resources";
	inline const std::string TEXT_HEADER_FACTION_STARTING_POPULATION = TEXT_HEADER_BEGINNING + "Faction Starting Population";
	constexpr glm::vec3 TERRAIN_POSITION = { 0.0f, -0.01f, 0.0f };
	constexpr size_t MAX_LEVELS = 5;
	constexpr unsigned int INVALID_OPENGL_ID = 0;
	constexpr float GROUND_HEIGHT = 0.0f;
	constexpr int MAP_SIZE = 30;
	constexpr int NODE_SIZE = 6;
	constexpr int INVALID_ENTITY_ID = -1;
	constexpr size_t MAX_MINERALS_PER_FACTION = 5;
	constexpr int MIN_FACTIONS = 2;
	constexpr int MAX_FACTIONS = 4;
	constexpr glm::vec3 PLAYER_MATERIAL_DIFFUSE = { 1.0f, 0.2f, 0.2f };
	constexpr glm::vec3 AI_1_MATERIAL_DIFFUSE = { 0.2f, 0.2f, 1.0f };
	constexpr glm::vec3 AI_2_MATERIAL_DIFFUSE = { 1.0f, 1.0f, 0.2f };
	constexpr glm::vec3 AI_3_MATERIAL_DIFFUSE = { 0.2f, 1.0f, 0.2f };
	inline const std::string FACTION_MATERIAL_NAME_ID = "metal";

	constexpr int HQ_STARTING_HEALTH = 25;
	constexpr int BARRACKS_STARTING_HEALTH = 10;
	constexpr int SUPPLY_DEPOT_STARTING_HEALTH = 10;
	constexpr int UNIT_STARTING_HEALTH = 5;
	constexpr int WORKER_STARTING_HEALTH = 2;

	constexpr int STARTING_RESOURCES = 1500;
	constexpr int STARTING_POPULATION = 50;
	constexpr int WORKER_RESOURCE_COST = 50;
	constexpr int SUPPLY_DEPOT_RESOURCE_COST = 50;
	constexpr int BARRACKS_RESOURCE_COST = 50;
	constexpr int UNIT_RESOURCE_COST = 100;
	constexpr int WORKER_POPULATION_COST = 1;
	constexpr int UNIT_POPULATION_COST = 2;
	constexpr int TURRET_RESOURCE_COST = 50;
	constexpr int POPULATION_INCREMENT = 5;

#ifdef GAME
	inline const TypeComparison<eEntityType> UNIT_SPAWNER_TYPES({ eEntityType::HQ, eEntityType::Barracks });
	inline const TypeComparison<eEntityType> BUILDING_TYPES({ eEntityType::HQ, eEntityType::Barracks, 
		eEntityType::SupplyDepot, eEntityType::Turret });
	inline const TypeComparison<eEntityType> UNIT_TYPES({ eEntityType::Unit, eEntityType::Worker });
#endif // GAME

	constexpr int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};

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
		float x = glm::pow(positionB.x - positionA.x, 2);
		float y = glm::pow(positionB.y - positionA.y, 2);

		return x + y;
	}

	inline float getSqrDistance(const glm::vec3& positionB, const glm::vec3& positionA)
	{
		float x = glm::pow(positionB.x - positionA.x, 2);
		float y = glm::pow(positionB.y - positionA.y, 2);
		float z = glm::pow(positionB.z - positionA.z, 2);

		return x + y + z;
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
		glm::vec3 newPosition = { std::floor(position.x), std::floor(position.y), std::floor(position.z) };
		int xDifference = static_cast<int>(position.x) % Globals::NODE_SIZE;
		if (xDifference > 0)
		{
			newPosition.x -= xDifference;
		}

		int zDifference = static_cast<int>(position.z) % Globals::NODE_SIZE;
		if (zDifference > 0)
		{
			newPosition.z -= zDifference;
		}

		return newPosition;
	}

	inline glm::vec3 convertToMiddleGridPosition(const glm::vec3& position)
	{
		assert(static_cast<int>(position.x) % Globals::NODE_SIZE == 0 &&
			position.y == Globals::GROUND_HEIGHT &&
			static_cast<int>(position.z) % Globals::NODE_SIZE == 0);

		return { position.x + static_cast<float>(Globals::NODE_SIZE) / 2.0f, position.y, 
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

	inline bool isEntityIDValid(int entityID)
	{
		return entityID != Globals::INVALID_ENTITY_ID;
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
}