#pragma once

#include <string>
#include <vector>

struct Player;
class Entity;
#ifdef LEVEL_EDITOR
class EntityManager;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{
#ifdef LEVEL_EDITOR
	void saveLevelToFile(const std::string& fileName, const EntityManager& entityManager, 
		const Player& player, const Player& playerAI);
	bool loadLevelFromFile(const std::string& fileName, EntityManager& entityManager,
		Player& player, Player& playerAI);
#endif // LEVEL_EDITOR

	bool loadLevelFromFile(const std::string& fileName, std::vector<Entity>& scenery);
}