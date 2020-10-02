#include "LevelFileHandler.h"
#ifdef LEVEL_EDITOR
#include "EntityManager.h"
#include "Player.h"
#include <ostream>
#endif // LEVEL_EDITOR
#ifdef GAME
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#endif // GAME
#include "Globals.h"
#include "Entity.h"
#include "ModelName.h"
#include <fstream>
#include <sstream>

namespace
{
	const std::string LEVELS_FILE_DIRECTORY = Globals::SHARED_FILE_DIRECTORY + "Levels/";
	const std::string LEVELS_FILE_NAME = "Levels.txt";
}

#ifdef GAME
void loadInPlayers(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	int startingResources, int startingPopulation);
void loadInPlayer(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	const FactionControllerDetails& factionControllerDetails, int startingResources, int startingPopulation);
void loadInScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery);
#endif // GAME

#ifdef LEVEL_EDITOR
void saveMapSizeToFile(std::ostream& os, const glm::ivec2& mapSize);
void saveFactionStartingResources(std::ostream& os, int factionStartingResources);
void saveFactionStartingPopulation(std::ostream& os, int factionStartingPopulation);
#endif // LEVEL_EDITOR

int loadFactionStartingResources(std::ifstream& file);
int loadFactionStartingPopulation(std::ifstream& file);
glm::ivec2 loadMapSizeFromFile(std::ifstream& file);
bool isPlayerActive(std::ifstream& file, eFactionController factionController);

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
bool LevelFileHandler::saveLevelToFile(const std::string& fileName, const EntityManager& entityManager,
	const std::vector<Player>& players, const glm::ivec2& mapSize, int factionStartingResources, 
	int factionStartingPopulation)
{
	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + fileName);
	assert(file.is_open());
	if (!file.is_open())
	{
		return false;
	}

	for (auto& player : players)
	{
		file << player;
	}
	
	saveFactionStartingPopulation(file, factionStartingPopulation);
	saveFactionStartingResources(file, factionStartingResources);
	saveMapSizeToFile(file, mapSize);

	file << entityManager;
	file.close();

	return true;
}

bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, EntityManager& entityManager, std::vector<Player>& players, 
	glm::ivec2& mapSize, int& factionStartingResources, int& factionStartingPopulation)
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}
	
	for (const auto& factionControllerDetails : FACTION_CONTROLLER_DETAILS)
	{
		switch (factionControllerDetails.controller)
		{
		case eFactionController::Player:	
		case eFactionController::AI_1:
			assert(isPlayerActive(file, factionControllerDetails.controller));
			players.emplace_back(factionControllerDetails.controller);
			break;
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			if (isPlayerActive(file, factionControllerDetails.controller))
			{
				players.emplace_back(factionControllerDetails.controller);
			}
			break;
		default:
			assert(false);
		}
	}

	mapSize = loadMapSizeFromFile(file);
	factionStartingResources = loadFactionStartingResources(file);
	factionStartingPopulation = loadFactionStartingPopulation(file);

	for (auto& player : players)
	{
		file >> player;
	}

	file >> entityManager;
	file.close();

	return true;
}

std::array<std::string, Globals::MAX_LEVELS> LevelFileHandler::loadLevelNames()
{
	std::array<std::string, Globals::MAX_LEVELS> levelNames;
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY);
	if (!file.is_open())
	{
		std::ofstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + LEVELS_FILE_NAME);
		assert(file.is_open());
	}
	else
	{
		std::string line;
		int index = 0;
		while (getline(file, line))
		{
			std::stringstream stringstream{ line };
			stringstream >> levelNames[index];
			++index;

			assert(index <= static_cast<int>(Globals::MAX_LEVELS));
		}
	}

	return levelNames;
}

void saveMapSizeToFile(std::ostream& os, const glm::ivec2& mapSize)
{
	os << Globals::TEXT_HEADER_MAP_SIZE << "\n";
	os << mapSize.x << " " << mapSize.y << "\n";
}

void saveFactionStartingResources(std::ostream& os, int factionStartingResources)
{
	os << Globals::TEXT_HEADER_FACTION_STARTING_RESOURCE << "\n";
	os << factionStartingResources << "\n";
}

void saveFactionStartingPopulation(std::ostream& os, int factionStartingPopulation)
{
	os << Globals::TEXT_HEADER_FACTION_STARTING_POPULATION << "\n";
	os << factionStartingPopulation << "\n";
}
#endif // LEVEL_EDITOR

int loadFactionStartingResources(std::ifstream& file)
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

int loadFactionStartingPopulation(std::ifstream& file)
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

glm::ivec2 loadMapSizeFromFile(std::ifstream& file)
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

bool isPlayerActive(std::ifstream& file, eFactionController factionController)
{
	assert(file.is_open());
	bool playerFound = false;
	std::string line;
	while (getline(file, line))
	{
		if (line == FACTION_CONTROLLER_DETAILS[static_cast<int>(factionController)].text)
		{
			playerFound = true;
			break;
		}
	}

	file.clear();
	file.seekg(0);

	return playerFound;
}

#ifdef GAME
bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery, 
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions)
{
	assert(scenery.empty());

	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + LEVELS_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	glm::ivec2 mapSize = loadMapSizeFromFile(file);
	GameMessenger::getInstance().broadcast<GameMessages::NewMapSize>({ mapSize });
	int factionStartingResources = loadFactionStartingResources(file);
	int factionStartingPopulation = loadFactionStartingPopulation(file);
	loadInPlayers(file, factions, factionStartingResources, factionStartingPopulation);
	loadInScenery(file, scenery);
	
	return true;
}

void loadInPlayers(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	int startingResources, int startingPopulation)
{
	for (const auto& factionControllerDetails : FACTION_CONTROLLER_DETAILS)
	{
		switch (factionControllerDetails.controller)
		{
		case eFactionController::Player:
		case eFactionController::AI_1:
			assert(isPlayerActive(file, factionControllerDetails.controller));
			loadInPlayer(file, factions, factionControllerDetails, startingResources, startingPopulation);
			break;
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			if (isPlayerActive(file, factionControllerDetails.controller))
			{
				loadInPlayer(file, factions, factionControllerDetails, startingResources, startingPopulation);
			}
			break;
		default:
			assert(false);
		}
	}
}

void loadInPlayer(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	const FactionControllerDetails& factionControllerDetails, int startingResources, int startingPopulation)
{
	assert(file.is_open());
	glm::vec3 hqStartingPosition = { 0.0f, 0.0f, 0.0f };
	std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION> mineralPositions;

	auto data = [&hqStartingPosition, &mineralPositions](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string rawModelName;
		glm::vec3 position;
		stream >> rawModelName >> position.x >> position.y >> position.z;
		switch (static_cast<eModelName>(std::stoi(rawModelName)))
		{
		case eModelName::HQ:
			hqStartingPosition = position;
			break;
		case eModelName::Mineral:
		{
			int mineralIndex = -1;
			stream >> mineralIndex;
			assert(mineralIndex >= 0 && mineralIndex < Globals::MAX_MINERALS_PER_FACTION);

			mineralPositions[mineralIndex] = position;
		}
		break;
		}
	};

	const std::string& text = factionControllerDetails.text;
	auto conditional = [&text](const std::string& line)
	{
		return line == text;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	switch (factionControllerDetails.controller)
	{
	case eFactionController::Player:
		factions[static_cast<int>(factionControllerDetails.controller)] = std::make_unique<FactionPlayer>(factionControllerDetails.controller,
			hqStartingPosition, mineralPositions, startingResources, startingPopulation);
		break;
	case eFactionController::AI_1:
	case eFactionController::AI_2:
	case eFactionController::AI_3:
		factions[static_cast<int>(factionControllerDetails.controller)] = std::make_unique<FactionAI>(factionControllerDetails.controller,
			hqStartingPosition, mineralPositions, startingResources, startingPopulation);
		break;
	default:
		assert(false);
	}
}

void loadInScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery)
{
	assert(file.is_open() && scenery.empty());

	auto data = [&scenery](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string rawModelName;
		glm::vec3 position;
		stream >> rawModelName >> position.x >> position.y >> position.z;

		scenery.emplace_back(static_cast<eModelName>(std::stoi(rawModelName)), position);
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_SCENERY;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);
}
#endif // GAME