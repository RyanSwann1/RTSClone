#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameObjectManager.h"
#include "Quad.h"
#include "Globals.h"
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

struct BaseLocation
{
	BaseLocation(const glm::vec3& position);

	void setPosition(const glm::vec3& position);

	Quad quad;
	std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION> minerals;
};

struct Camera;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	static std::unique_ptr<Level> load(const std::string& levelName);

	const std::string& getName() const;

	void handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime, glm::uvec2 windowSize);
	void handleModelNamesGUI();
	void handlePlayersGUI();
	void handleSelectedEntityGUI();
	void handleLevelDetailsGUI(bool& showGUIWindow);
	void handleMainBasesGui();
	void save() const;

	void render(ShaderHandler& shaderHandler) const;
	void renderDebug(ShaderHandler& shaderHandler) const; 

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	friend const std::ifstream& operator>>(std::ifstream& file, Level& level);
	friend std::ostream& operator<<(std::ostream& file , const Level& level);
private:
	Level(const std::string& levelName);

	const std::string m_levelName;
	std::vector<BaseLocation> m_mainBaseLocations;
	PlannedEntity m_plannedEntity;
	glm::ivec2 m_size;
	Quad m_playableArea;
	GameObjectManager m_gameObjectManager;
	GameObject* m_selectedGameObject;
	BaseLocation* m_selectedBaseLocation;
	Mineral* m_selectedMineral;
	int m_factionStartingResources;
	int m_factionStartingPopulationCap;
	int m_factionCount;
};