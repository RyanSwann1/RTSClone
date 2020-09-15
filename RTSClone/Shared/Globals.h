#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "FactionController.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <random>
#include <string>

namespace Globals
{ 
	const std::string SHARED_FILE_DIRECTORY = "../Shared/";
	const std::string TEXT_HEADER_BEGINNING = "#";
	//const std::string TEXT_HEADER_PLAYER = TEXT_HEADER_BEGINNING + "Player";
	//const std::string TEXT_HEADER_PLAYERAI_1 = TEXT_HEADER_BEGINNING + "PlayerAI_1";
	//const std::string TEXT_HEADER_PLAYERAI_2 = TEXT_HEADER_BEGINNING + "PlayerAI_2";
	//const std::string TEXT_HEADER_PLAYERAI_3 = TEXT_HEADER_BEGINNING + "PlayerAI_3";
	const std::string TEXT_HEADER_SCENERY = TEXT_HEADER_BEGINNING + "Scenery";
	constexpr unsigned int INVALID_OPENGL_ID = 0;
	constexpr float GROUND_HEIGHT = 0.0f;
	constexpr int MAP_SIZE = 30;
	constexpr int NODE_SIZE = 6;
	constexpr int INVALID_ENTITY_ID = -1;
	constexpr size_t MAX_MINERALS_PER_FACTION = 5;
	constexpr int MIN_FACTIONS = 2;
	constexpr int MAX_FACTIONS = 4;

	constexpr int STARTING_RESOURCES = 500;
	constexpr int WORKER_RESOURCE_COST = 50;
	constexpr int SUPPLY_DEPOT_RESOURCE_COST = 50;
	constexpr int BARRACKS_RESOURCE_COST = 50;
	constexpr int UNIT_RESOURCE_COST = 100;
	constexpr int STARTING_POPULATION = 5;
	constexpr int MAX_POPULATION = 20;
	constexpr int WORKER_POPULATION_COST = 1;
	constexpr int UNIT_POPULATION_COST = 2;
	constexpr int POPULATION_INCREMENT = 5;

	constexpr int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};

	inline bool isWithinMapBounds(const AABB& AABB)
	{
		return AABB.getLeft() >= 0 &&
			AABB.getRight() < Globals::MAP_SIZE * Globals::NODE_SIZE&&
			AABB.getBack() >= 0 &&
			AABB.getForward() < Globals::MAP_SIZE * Globals::NODE_SIZE;
	}

	inline bool isPositionInMapBounds(const glm::ivec2& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE;
	}

	inline bool isPositionInMapBounds(const glm::vec3& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE * Globals::NODE_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE * Globals::NODE_SIZE &&
			position.z >= 0 &&
			position.z < Globals::MAP_SIZE * Globals::NODE_SIZE;
	}

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
		return static_cast<int>(position.x) % Globals::NODE_SIZE == 0 &&
			position.y == Globals::GROUND_HEIGHT &&
			static_cast<int>(position.z) % Globals::NODE_SIZE == 0;
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

	inline int convertTo1D(const glm::ivec2& position)
	{
		return position.x * Globals::MAP_SIZE + position.y;
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
}