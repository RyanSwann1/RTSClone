#include "LevelFileHandler.h"
#ifdef LEVEL_EDITOR
#include "GameObjectManager.h"
#endif // LEVEL_EDITOR
#include "Globals.h"
#include "Entity.h"
#include "ModelName.h"
#include <fstream>
#include <sstream>

#ifdef LEVEL_EDITOR
void LevelFileHandler::saveLevelToFile(const std::string& fileName, const GameObjectManager& gameObjectManager)
{
	std::stringstream stringStream;
	
	for (const auto& gameObject : gameObjectManager.getEntities())
	{
		stringStream << static_cast<int>(gameObject.getModelName()) << " " <<
			gameObject.getPosition().x << " " << gameObject.getPosition().y << " " << gameObject.getPosition().z << "\n";
	}

	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + fileName);
	file << stringStream.str();
	file.close();
	stringStream.clear();
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