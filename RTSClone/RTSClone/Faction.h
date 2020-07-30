#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Unit.h"
#include "Headquarters.h"
#include <SFML/Graphics.hpp>

struct SelectionBox : private NonMovable, private NonCopyable
{
	SelectionBox();
	~SelectionBox();

	AABB AABB;
	bool active;
	glm::vec3 mouseToGroundPosition;
	glm::vec2 startingPositionScreenPosition;
	glm::vec3 startingPositionWorldPosition;
	unsigned int vaoID;
	unsigned int vboID;
};

class ShaderHandler;
class Model;
struct Camera;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	Faction(const Model& headquartersModel, const Model& unitModel, Map& map);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler, const Model& hqModel, const Model& unitModel,
		const Model& waypointModel) const;
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
	Unit m_unit;
};