#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <array>

class SceneryGameObject;
class Faction;
struct Player;
class Entity;
#ifdef LEVEL_EDITOR
class EntityManager;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{
	void loadFromFile(std::ifstream& file, const std::function<void(const std::string&)>& data,
		const std::function<bool(const std::string&)>& conditional);

#ifdef LEVEL_EDITOR
	bool saveLevelToFile(const std::string& fileName, const EntityManager& entityManager,
		const std::vector<Player>& players, const glm::ivec2& mapSize, int factionStartingResources, 
		int factionStartingPopulation);
	bool loadLevelFromFile(const std::string& fileName, EntityManager& entityManager,
		std::vector<Player>& players, glm::ivec2& mapSize);
#endif // LEVEL_EDITOR

#ifdef GAME
	bool loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery,
		std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);
#endif // GAME
};