#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"
#include "Globals.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <array>
#include <fstream>

class SceneryGameObject;
class Faction;
struct Player;
class Entity;
#ifdef LEVEL_EDITOR
class LevelNameGenerator;
class EntityManager;
class Level;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{ 
	int loadFactionStartingResources(std::ifstream& file);
	int loadFactionStartingPopulation(std::ifstream& file);
	glm::ivec2 loadMapSizeFromFile(std::ifstream& file); 
	bool isPlayerActive(std::ifstream& file, eFactionController factionController);
	void loadFromFile(std::ifstream& file, const std::function<void(const std::string&)>& data,
		const std::function<bool(const std::string&)>& conditional);

#ifdef LEVEL_EDITOR
	bool isLevelExists(const std::string& fileName);
	void saveLevelName(const std::string& fileName);
	bool saveLevelToFile(const Level& level);
	bool loadLevelFromFile(Level& level);
#endif // LEVEL_EDITOR

#ifdef GAME
	bool loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery,
		std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);
#endif // GAME
};