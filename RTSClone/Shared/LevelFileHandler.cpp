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

#ifdef GAME
void loadInPlayers(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);
void loadInPlayer(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	const FactionControllerDetails& factionControllerDetails);
void loadInScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery);
#endif // GAME

#ifdef LEVEL_EDITOR
void saveMapSizeToFile(std::ostream& os, const glm::ivec2& mapSize);
void saveFactionStartingResources(std::ostream& os, int factionStartingResources);
void saveFactionStartingPopulation(std::ostream& os, int factionStartingPopulation);
#endif // LEVEL_EDITOR

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
	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
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
	glm::ivec2& mapSize)
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
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

	for (auto& player : players)
	{
		file >> player;
	}

	file >> entityManager;
	file.close();

	return true;
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

	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	GameMessenger::getInstance().broadcast<GameMessages::NewMapSize>({ loadMapSizeFromFile(file) });
	loadInPlayers(file, factions);
	loadInScenery(file, scenery);
	
	return true;
}

void loadInPlayers(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions)
{
	for (const auto& factionControllerDetails : FACTION_CONTROLLER_DETAILS)
	{
		switch (factionControllerDetails.controller)
		{
		case eFactionController::Player:
		case eFactionController::AI_1:
			assert(isPlayerActive(file, factionControllerDetails.controller));
			loadInPlayer(file, factions, factionControllerDetails);
			break;
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			if (isPlayerActive(file, factionControllerDetails.controller))
			{
				loadInPlayer(file, factions, factionControllerDetails);
			}
			break;
		default:
			assert(false);
		}
	}
}

void loadInPlayer(std::ifstream& file, std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions,
	const FactionControllerDetails& factionControllerDetails)
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
			hqStartingPosition, mineralPositions);
		break;
	case eFactionController::AI_1:
	case eFactionController::AI_2:
	case eFactionController::AI_3:
		factions[static_cast<int>(factionControllerDetails.controller)] = std::make_unique<FactionAI>(factionControllerDetails.controller,
			hqStartingPosition, mineralPositions);
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