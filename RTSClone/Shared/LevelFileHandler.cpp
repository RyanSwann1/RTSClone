#include "LevelFileHandler.h"
#ifdef LEVEL_EDITOR
#include "EntityManager.h"
#endif // LEVEL_EDITOR
#include "Globals.h"
#include "Entity.h"
#include "ModelName.h"
#include "Player.h"
#include <fstream>
#include <sstream>

#ifdef LEVEL_EDITOR
void LevelFileHandler::saveLevelToFile(const std::string& fileName, const EntityManager& entityManager,
	const Player& player, const Player& playerAI)
{
	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	file << player;
	file << playerAI;
	file << entityManager;
	
	file.close();
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

bool LevelFileHandler::loadLevelFromFile(const std::string& fileName, std::vector<Entity>& scenery)
{
	assert(scenery.empty());

	std::ifstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	if (!file.is_open())
	{
		return false;
	}

	std::string line;
	while(getline(file, line))
	{
		std::stringstream stream{ line };
		std::string rawModelName;
		glm::vec3 position;
		stream >> rawModelName >> position.x >> position.y >> position.z;
	
		scenery.emplace_back(static_cast<eModelName>(std::stoi(rawModelName)), position);
	}

	return true;
}