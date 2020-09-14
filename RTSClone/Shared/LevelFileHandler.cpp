#include "LevelFileHandler.h"
#ifdef LEVEL_EDITOR
#include "EntityManager.h"
#include "Player.h"
#endif // LEVEL_EDITOR
#ifdef GAME
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#endif // GAME
#include "Globals.h"
#include "Entity.h"
#include "ModelName.h"
#include <fstream>
#include <sstream>

#ifdef GAME
void loadInPlayer(std::ifstream& file, std::vector<std::unique_ptr<Faction>>& factions, const std::string& textHeaderFile);
void loadInScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery);
#endif // GAME

#ifdef LEVEL_EDITOR
bool LevelFileHandler::saveLevelToFile(const std::string& fileName, const EntityManager& entityManager,
	const Player& player, const Player& playerAI)
{
	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	assert(file.is_open());
	if (!file.is_open())
	{
		return false;
	}

	file << player;
	file << playerAI;
	file << entityManager;
	file.close();

	return true;
}

bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, EntityManager& entityManager, Player& player, Player& playerAI)
{
	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	file >> player;
	file >> playerAI;
	file >> entityManager;
	file.close();

	return true;
}
#endif // LEVEL_EDITOR

#ifdef GAME
bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery, 
	std::vector<std::unique_ptr<Faction>>& factions)
{
	assert(scenery.empty());

	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	loadInPlayer(file, factions, Globals::TEXT_HEADER_PLAYER);
	loadInPlayer(file, factions, Globals::TEXT_HEADER_PLAYERAI);
	loadInScenery(file, scenery);

	return true;
}

void loadInPlayer(std::ifstream& file, std::vector<std::unique_ptr<Faction>>& factions, const std::string& textHeaderFile)
{
	assert(file.is_open());
	bool beginReadingFromFile = false;
	std::string line;
	glm::vec3 hqStartingPosition = { 0.0f, 0.0f, 0.0f };
	std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION> mineralPositions;
	while (getline(file, line))
	{
		if (beginReadingFromFile)
		{
			if (line[0] == *Globals::TEXT_HEADER_BEGINNING.c_str())
			{
				file.clear();
				file.seekg(0);
				break;
			}

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
		}
		else if (line == textHeaderFile)
		{
			assert(!beginReadingFromFile);
			beginReadingFromFile = true;
		}
	}

	assert(beginReadingFromFile);
	if (textHeaderFile == Globals::TEXT_HEADER_PLAYER)
	{
		factions.emplace_back(std::make_unique<FactionPlayer>(eFactionName::Player, hqStartingPosition, mineralPositions));
	}
	else if (textHeaderFile == Globals::TEXT_HEADER_PLAYERAI)
	{
		factions.emplace_back(std::make_unique<FactionAI>(eFactionName::AI, hqStartingPosition, mineralPositions));
	}
}

void loadInScenery(std::ifstream& file, std::vector<SceneryGameObject>& scenery)
{
	assert(file.is_open() && scenery.empty());

	bool beginReadingFromFile = false;
	std::string line;
	while (getline(file, line))
	{
		if (beginReadingFromFile)
		{
			if (line[0] == *Globals::TEXT_HEADER_BEGINNING.c_str())
			{
				file.clear();
				file.seekg(0);
				break;
			}

			std::stringstream stream{ line };
			std::string rawModelName;
			glm::vec3 position;
			stream >> rawModelName >> position.x >> position.y >> position.z;

			scenery.emplace_back(static_cast<eModelName>(std::stoi(rawModelName)), position);
		}
		else if (line == Globals::TEXT_HEADER_SCENERY)
		{
			assert(!beginReadingFromFile);
			beginReadingFromFile = true;
		}
	}

	assert(beginReadingFromFile);
}
#endif // GAME