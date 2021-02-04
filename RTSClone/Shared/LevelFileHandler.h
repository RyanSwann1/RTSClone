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

class Faction;
struct Base;
class SceneryGameObject;
#ifdef LEVEL_EDITOR
class Level;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{ 
	void loadAllMainBases(std::ifstream& file, std::vector<Base>& mainBases);
	int loadFactionCount(std::ifstream& file);
	int loadFactionStartingPopulation(std::ifstream& file);
	int loadFactionStartingResources(std::ifstream& file);
	glm::ivec2 loadMapSizeFromFile(std::ifstream& file);

	void loadFromFile(std::ifstream& file, const std::function<void(const std::string&)>& data,
		const std::function<bool(const std::string&)>& conditional);

#ifdef LEVEL_EDITOR
	bool isLevelExists(const std::string& fileName);
	void saveLevelName(const std::string& fileName);
	bool saveLevelToFile(const Level& level);
	bool loadLevelFromFile(Level& level);
	void removeLevel(const std::string& fileName);
#endif // LEVEL_EDITOR

#ifdef GAME
	bool loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery,
		std::vector<Base>& mainBases, int& factionStartingResources,
		int& factionStartingPopulation, int& factionCount, glm::vec3& size);

	std::array<std::string, Globals::MAX_LEVELS> loadLevelNames();
#endif // GAME
};