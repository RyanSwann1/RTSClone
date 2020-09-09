#pragma once

#include "Faction.h"
#include "SelectionBox.h"
#include <SFML/Graphics.hpp>

struct Camera;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		const Faction& opposingFaction);
	void renderSelectionBox(const sf::Window& window) const;

private:
	SelectionBox m_selectionBox;
	glm::vec3 m_previousMouseToGroundPosition;
	bool m_attackMoveSelected;

	bool isOneUnitSelected() const;
	
	void moveSingularSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map);
	void moveMultipleSelectedUnits(const glm::vec3& mouseToGroundPosition, const Map& map);
	void instructWorkerReturnMinerals(const Map& map);
	void instructUnitToAttack(Unit& Unit, int targetEntityID, const Faction& opposingFaction, const Map& map);
	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map);

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