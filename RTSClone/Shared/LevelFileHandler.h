#pragma once

#include <string>
#include <vector>

struct GameObject;
#ifdef LEVEL_EDITOR
class GameObjectManager;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{
#ifdef LEVEL_EDITOR
	void saveLevelToFile(const std::string& fileName, const GameObjectManager& gameObjectManager);
#endif // LEVEL_EDITOR

	bool loadLevelFromFile(const std::string& fileName, std::vector<GameObject>& scenery);
}