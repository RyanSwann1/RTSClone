#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Player.h"
#include "EntityManager.h"
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct Camera;
class ShaderHandler;
class PlayableAreaDisplay;
class SelectionBox;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	static std::unique_ptr<Level> load(const std::string& levelName);

	const std::string& getName() const;
	const std::vector<Player>& getPlayers() const;
	const glm::ivec2& getMapSize() const;
	const EntityManager& getEntityManager() const;

	void addEntity(eModelName modelName, const glm::vec3& position);
	void handleInput(const sf::Event& currentSFMLEvent, const SelectionBox& selectionBox, const Camera& camera, 
		bool plannedEntityActive, const sf::Window& window, const Entity& plannedEntity);
	void handlePlayerDetails(bool& showGUIWindow);
	void handleLevelDetails(bool& showGUIWindow, PlayableAreaDisplay& playableAreaDisplay);
	void save() const;
	void render(ShaderHandler& shaderHandler) const;

	friend const std::ifstream& operator>>(std::ifstream& file, Level& level);
	friend std::ostream& operator<<(std::ostream& ostream, const Level& level);

private:
	Level(const std::string& levelName);

	const std::string m_levelName;
	EntityManager m_entityManager;
	std::vector<Player> m_players;
	glm::ivec2 m_mapSize;
	int m_factionStartingResources;
	int m_factionStartingPopulation;
};