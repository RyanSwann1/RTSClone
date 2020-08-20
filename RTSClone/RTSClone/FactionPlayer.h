#pragma once

#include "Faction.h"
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

struct Camera;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Map& map, float deltaTime);
	void renderSelectionBox(const sf::Window& window) const;

private:
	SelectionBox m_selectionBox;
	glm::vec3 m_previousMouseToGroundPosition;

	bool isOneUnitSelected() const;
	
	void moveSingularSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map);
	void moveMultipleSelectedUnits(const glm::vec3& mouseToGroundPosition, const Map& map);
	void instructWorkerReturnMinerals(const Map& map);

	template <class Entity>
	void selectUnit(std::list<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllUnits)
	{
		auto selectedEntity = std::find_if(entities.begin(), entities.end(), [&mouseToGroundPosition](const auto& entity)
		{
			return entity.getAABB().contains(mouseToGroundPosition);
		});
		if (selectedEntity != entities.cend())
		{
			if (selectAllUnits)
			{
				for (auto& entity : entities)
				{
					entity.setSelected(true);
				}
			}
			else
			{
				for (auto& entity : entities)
				{
					entity.setSelected(entity.getAABB().contains(mouseToGroundPosition));
				}
			}
		}
		else
		{
			for (auto& entity : entities)
			{
				entity.setSelected(false);
			}
		}
	}
};