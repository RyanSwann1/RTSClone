#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Headquarters.h"
#include "Harvester.h"
#include <SFML/Graphics.hpp>

struct SelectionBox : private NonMovable, private NonCopyable
{
	SelectionBox();
	~SelectionBox();

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void setSize(const glm::vec3& position);
	void reset();
	void render(const sf::Window& window) const;

	AABB AABB;
	bool active;
	glm::vec3 mouseToGroundPosition;
	glm::vec2 startingPositionScreenPosition;
	glm::vec3 startingPositionWorldPosition;
	unsigned int vaoID;
	unsigned int vboID;
};

class ShaderHandler;
class ModelManager;
struct Camera;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	Faction(const ModelManager& modelManager, Map& map);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Map& map, 
		const ModelManager& modelManager, const std::vector<Entity>& minerals);
	void update(float deltaTime, const ModelManager& modelManager, const Map& map);
	void render(ShaderHandler& shaderHandler, const ModelManager& modelManager) const;
	void renderSelectionBox(const sf::Window& window) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	SelectionBox m_selectionBox;
	Headquarters m_HQ;
	std::vector<Unit> m_units;
	std::vector<Harvester> m_harvesters;

	void spawnUnit(const glm::vec3& spawnPosition, const Model& unitModel, Map& map);
	void spawnHarvester(const glm::vec3& spawnPosition, const Model& unitModel, Map& map);
	bool isOneUnitSelected() const;
	void moveSingularSelectedUnit(const glm::vec3& destinationPosition, const Map& map, const std::vector<Entity>& minerals);
	void moveMultipleSelectedUnits(const glm::vec3& destinationPosition, const Map& map, const std::vector<Entity>& minerals);
	void handleHarvesterCollisions(const Map& map);
};