#pragma once

#include <string>
#include <vector>
#include <memory>

class SceneryGameObject;
class Faction;
struct Player;
class Entity;
#ifdef LEVEL_EDITOR
class EntityManager;
#endif // LEVEL_EDITOR
namespace LevelFileHandler
{
#ifdef LEVEL_EDITOR
	bool saveLevelToFile(const std::string& fileName, const EntityManager& entityManager, 
		const Player& player, const Player& playerAI);
	bool loadLevelFromFile(const std::string& fileName, EntityManager& entityManager,
		Player& player, Player& playerAI);
#endif // LEVEL_EDITOR

#ifdef GAME
	bool loadLevelFromFile(const std::string& fileName, std::vector<SceneryGameObject>& scenery, 
		std::vector<std::unique_ptr<Faction>>& factions);
#endif // GAME
}