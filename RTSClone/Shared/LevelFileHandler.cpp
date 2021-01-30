#include "LevelFileHandler.h"
#ifdef LEVEL_EDITOR
#include "Level.h"
#include <ostream>
#endif // LEVEL_EDITOR
#ifdef GAME
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "Level.h"
#endif // GAME
#include "Globals.h"
#include "ModelManager.h"
#include <fstream>
#include <sstream>

namespace
{
	const std::string LEVELS_FILE_DIRECTORY = Globals::SHARED_FILE_DIRECTORY + "Levels/";
	const std::string LEVELS_FILE_NAME = "Levels.txt";
}

#ifdef GAME
void loadScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery);
int loadMainBaseQuantity(std::ifstream& file);
void loadAllMainBases(std::ifstream& file, std::vector<Base>& mainBases);
void loadMainBase(std::ifstream& file, const std::string& textHeader, glm::vec3& position);
void loadMainBaseMinerals(std::ifstream& file, const std::string& textHeader, std::vector<Mineral>& minerals);
void loadFactionStartingResources(std::ifstream& file, int& factionStartingResources);
void loadFactionStartingPopulation(std::ifstream& file, int& factionStartingPopulation);
#endif // GAME

void LevelFileHandler::loadFromFile(std::ifstream& file, const std::function<void(const std::string&)>& data, 
	const std::function<bool(const std::string&)>& conditional)
{
	assert(file.is_open() && data && conditional);
	bool beginReadingFromFile = false;
	std::string line;
	while (getline(file, line))
	{
		if (beginReadingFromFile)
		{
			assert(!line.empty());
			if (line[0] == *Globals::TEXT_HEADER_BEGINNING.c_str())
			{
				break;
			}

			data(line);
		}
		else if (conditional(line))
		{
			assert(!beginReadingFromFile);
			beginReadingFromFile = true;
		}
	}
	
	file.clear();
	file.seekg(0);
	assert(beginReadingFromFile);
}

#ifdef LEVEL_EDITOR
bool LevelFileHandler::isLevelExists(const std::string& fileName)
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME);
	assert(file.is_open());

	std::string line;
	while (std::getline(file, line))
	{
		if (line == fileName)
		{
			return true;
		}
	}

	return false;
}

void LevelFileHandler::saveLevelName(const std::string& fileName)
{
	assert(!isLevelExists(fileName));
	std::ofstream levelsFile(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME, std::fstream::in | std::fstream::out | std::fstream::app);
	levelsFile << fileName << "\n";
}

bool LevelFileHandler::saveLevelToFile(const Level& level)
{
	assert(isLevelExists(level.getName()));
	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + level.getName());
	assert(file.is_open());
	if (!file.is_open())
	{
		return false;
	}

	file << level;

	return true;
}

bool LevelFileHandler::loadLevelFromFile(Level& level)
{
	assert(isLevelExists(level.getName()));
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + level.getName());
	if (!file.is_open())
	{
		return false;
	}
	
	file >> level;

	file.close();

	return true;
}

void LevelFileHandler::removeLevel(const std::string& fileName)
{
	assert(isLevelExists(fileName));

	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME);
	assert(file.is_open());

	std::ofstream tempFile(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + "temp.txt");
	assert(tempFile.is_open());

	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream stream{ line };
		if (stream.str() != fileName)
		{
			tempFile << stream.str() << "\n";
		}
	}

	file.close();
	tempFile.close();

	std::remove(std::string(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME).c_str());

	std::rename(std::string(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + "temp.txt").c_str(),
		std::string(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME).c_str());

	std::remove(std::string(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + "temp.txt").c_str());

	std::remove(std::string(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + fileName).c_str());
}

int LevelFileHandler::loadFactionStartingResources(std::ifstream& file)
{
	int factionStartingResources = 0;
	auto data = [&factionStartingResources](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> factionStartingResources;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_FACTION_STARTING_RESOURCE;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return factionStartingResources;
}

int LevelFileHandler::loadFactionStartingPopulationCap(std::ifstream& file)
{
	int factionStartingPopulation = 0;
	auto data = [&factionStartingPopulation](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> factionStartingPopulation;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_FACTION_STARTING_POPULATION;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return factionStartingPopulation;
}
#endif // LEVEL_EDITOR

glm::ivec2 LevelFileHandler::loadMapSizeFromFile(std::ifstream& file)
{
	glm::ivec2 mapSize = { 0, 0 };
	auto data = [&mapSize](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> mapSize.x >> mapSize.y;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_MAP_SIZE;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return mapSize;
}

#ifdef GAME
bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery,
	std::vector<Base>& mainBases, int& factionStartingResources,
	int& factionStartingPopulation)
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	broadcastToMessenger<GameMessages::NewMapSize>({ loadMapSizeFromFile(file) });
	loadScenery(file, scenery);
	loadFactionStartingResources(file, factionStartingResources);
	loadFactionStartingPopulation(file, factionStartingPopulation);
	loadAllMainBases(file, mainBases);

	return true;
}

std::array<std::string, Globals::MAX_LEVELS> LevelFileHandler::loadLevelNames()
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME);
	assert(file.is_open());
	if (!file.is_open())
	{
		//Create level names files file		
	}

	std::array<std::string, Globals::MAX_LEVELS> levelNames;
	std::string line;
	int lineCount = 0;
	while (std::getline(file, line))
	{
		assert(!line.empty() && lineCount < Globals::MAX_LEVELS);
		std::stringstream stream{ line };

		levelNames[lineCount] = stream.str();
		++lineCount;
	}

	return levelNames;
}

void loadScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery)
{
	assert(file.is_open() && scenery.empty());

	auto data = [&scenery](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string modelName;
		glm::vec3 rotation;
		glm::vec3 position;
		stream >> modelName >> rotation.x >> rotation.y >> rotation.z >> position.x >> position.y >> position.z;

		scenery.emplace_back(ModelManager::getInstance().getModel(modelName), position, rotation);
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_SCENERY;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
}

int loadMainBaseQuantity(std::ifstream& file)
{
	assert(file.is_open());

	int mainBaseQuantity = 0;
	auto data = [&mainBaseQuantity](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> mainBaseQuantity;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_MAIN_BASE_QUANTITY;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
	return mainBaseQuantity;
}

void loadAllMainBases(std::ifstream& file, std::vector<Base>& mainBases)
{
	assert(file.is_open());

	for (int i = 0; i < loadMainBaseQuantity(file); ++i)
	{
		glm::vec3 mainBasePosition(0.0f);
		loadMainBase(file, Globals::TEXT_HEADER_MAIN_BASES[i], mainBasePosition);
		
		std::vector<Mineral> minerals;
		loadMainBaseMinerals(file, Globals::TEXT_HEADER_MAIN_BASE_MINERALS[i], minerals);

		mainBases.emplace_back(mainBasePosition, std::move(minerals));
	}
}

void loadMainBase(std::ifstream& file, const std::string& textHeader, glm::vec3& position)
{
	assert(file.is_open());

	auto data = [&position](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> position.x >> position.y >> position.z;
	};

	auto conditional = [&textHeader](const std::string& line)
	{
		return line == textHeader;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
}

void loadMainBaseMinerals(std::ifstream& file, const std::string& textHeader, std::vector<Mineral>& minerals)
{
	assert(file.is_open());

	auto data = [&minerals](const std::string& line)
	{
		std::stringstream stream{ line };
		glm::vec3 position;
		stream >> position.x >> position.y >> position.z;
		minerals.emplace_back(position);
	};

	auto conditional = [textHeader](const std::string& line)
	{
		return line == textHeader;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
}

void loadFactionStartingResources(std::ifstream& file, int& factionStartingResources)
{
	assert(file.is_open());

	auto data = [&factionStartingResources](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> factionStartingResources;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_FACTION_STARTING_RESOURCE;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

}

void loadFactionStartingPopulation(std::ifstream& file, int& factionStartingPopulation)
{
	assert(file.is_open());

	auto data = [&factionStartingPopulation](const std::string& line)
	{
		std::stringstream stream{ line };
		stream >> factionStartingPopulation;
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_FACTION_STARTING_POPULATION;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
}
#endif // GAME