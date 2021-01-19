#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Player.h"
#include "GameObjectManager.h"
#include "TranslateObject.h"
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct PlannedEntity
{
	PlannedEntity();

	int modelNameIDSelected;
	glm::vec3 position;
	const Model* model;
};

class ShaderHandler;
struct PlayableArea : private NonCopyable, private NonMovable
{
	PlayableArea(glm::ivec2 startingSize);
	~PlayableArea();

	void setSize(glm::ivec2 size);
	void render(ShaderHandler& shaderHandler) const;

	glm::ivec2 size;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;
};

struct Camera;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	static std::unique_ptr<Level> load(const std::string& levelName);

	glm::ivec2 getPlayableAreaSize() const;
	int getFactionStartingResources() const;
	int getFactionStartingPopulationCap() const;
	const std::string& getName() const;
	const std::vector<std::unique_ptr<Player>>& getPlayers() const;
	const GameObjectManager& getGameObjectManager() const;

	void handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime, glm::uvec2 windowSize);
	void handleModelNamesGUI();
	void handlePlayersGUI();
	void handleSelectedEntityGUI();
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
	PlayableArea m_playableArea;
	GameObjectManager m_gameObjectManager;
	std::vector<std::unique_ptr<Player>> m_players;
	Player* m_selectedPlayer;
	int m_factionStartingResources;
	int m_factionStartingPopulationCap;
};