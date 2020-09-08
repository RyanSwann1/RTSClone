#include "LevelFileSaver.h"
#include "GameObjectManager.h"
#include "Globals.h"
#include <fstream>
#include <sstream>

void LevelFileSaver::saveLevelToFile(const GameObjectManager& gameObjectManager)
{
	std::stringstream stringStream;

	for (const auto& gameObject : gameObjectManager.getGameObjects())
	{
		stringStream << static_cast<int>(gameObject.modelName) << "\n";
		stringStream << gameObject.position.x << " " << gameObject.position.y << " " << gameObject.position.z << "\n";
	}

	std::ofstream file(Globals::SHARED_FILE_DIRECTORY + "Level.txt");
	file << stringStream.str();
	file.close();
	stringStream.clear();
}