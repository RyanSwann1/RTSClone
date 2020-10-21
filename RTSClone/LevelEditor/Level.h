#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Player.h"
#include "EntityManager.h"
#include "TranslateObject.h"
#include "PlayableAreaDisplay.h"
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct PlannedEntity
{
	PlannedEntity();

	int modelNameIDSelected;
	glm::vec3 position;
	eModelName modelName;
	bool active;
};

struct Camera;
class ShaderHandler;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	static std::unique_ptr<Level> load(const std::string& levelName);

	int getFactionStartingResources() const;
	int getFactionStartingPopulation() const;
	const std::string& getName() const;
	const std::vector<Player>& getPlayers() const;
	const glm::ivec2& getMapSize() const;
	const EntityManager& getEntityManager() const;

	void addEntity(eModelName modelName, const glm::vec3& position);
	void handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime);
	void handleModelNamesGUI();
	void handlePlayersGUI();
	void handleLevelDetailsGUI(bool& showGUIWindow);
	void save() const;
	void render(ShaderHandler& shaderHandler) const;
	void renderPlayableArea(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	friend const std::ifstream& operator>>(std::ifstream& file, Level& level);

private:
	Level(const std::string& levelName);

	const std::string m_levelName;
	PlannedEntity m_plannedEntity;
	TranslateObject m_translateObject;
	PlayableAreaDisplay m_playableAreaDisplay;
	EntityManager m_entityManager;
	std::vector<Player> m_players;
	glm::ivec2 m_mapSize;
	int m_factionStartingResources;
	int m_factionStartingPopulation;
};