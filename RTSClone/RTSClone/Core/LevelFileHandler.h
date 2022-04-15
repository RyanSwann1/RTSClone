#pragma once

#include "Core/FactionController.h"
#include "glm/glm.hpp"
#include "Core/Globals.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <array>
#include <fstream>
#include <optional>

class Faction;
struct Base;
struct SceneryGameObject;
#ifdef LEVEL_EDITOR
class Level;
#endif // LEVEL_EDITOR
#ifdef GAME
#include "Core/Level.h"
#endif // GAME
namespace LevelFileHandler
{ 
	void loadAllMainBases(std::ifstream& file, std::vector<Base>& mainBases, int mineralQuantity);
	void loadAllSecondaryBases(std::ifstream& file, std::vector<Base>& secondaryBases, int mineralQuantity);
	int loadFactionCount(std::ifstream& file);
	int loadFactionStartingPopulation(std::ifstream& file);
	int loadFactionStartingResources(std::ifstream& file);
	int loadMineralQuantity(std::ifstream& file);
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
	std::optional<LevelDetailsFromFile> loadLevelFromFile(std::string_view fileName);
	std::array<std::string, Globals::MAX_LEVELS> loadLevelNames();
#endif // GAME
};