#pragma once

#include "GameObjectManager.h"
#include "Quad.h"
#include "Globals.h"
#include "Base.h"
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct PlannedEntity
{
	PlannedEntity();

	int modelNameIDSelected;
	glm::vec3 position;
	Model* model;
};

struct Camera;
class Level
{
public:
	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;
	Level(Level&&) = delete;
	Level& operator=(Level&&) = delete;
	static std::unique_ptr<Level> create(const std::string& levelName);
	static std::unique_ptr<Level> load(const std::string& levelName);

	const std::string& getName() const;

	void handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime, glm::uvec2 windowSize);
	void handleModelNamesGUI();
	void handlePlayersGUI();
	void handleSelectedEntityGUI();
	void handleLevelDetailsGUI(bool& showGUIWindow);
	void handleMainBasesGui();
	void handleSecondaryBaseGUI();
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
	std::vector<Base> m_mainBases;
	std::vector<Base> m_secondaryBases;
	PlannedEntity m_plannedEntity;
	glm::ivec2 m_size;
	Quad m_playableArea;
	GameObjectManager m_gameObjectManager;
	GameObject* m_selectedGameObject;
	Base* m_selectedBase;
	Mineral* m_selectedMineral;
	int m_factionStartingResources;
	int m_factionStartingPopulationCap;
	int m_factionCount;
};