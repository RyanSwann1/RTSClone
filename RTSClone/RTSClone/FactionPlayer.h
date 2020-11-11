#pragma once

#include "Faction.h"
#include "SelectionBox.h"
#include <SFML/Graphics.hpp>

class TargetEntity;
struct Camera;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
		int startingPopulationCap);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		const std::vector<const Faction*>& opposingFactions, TargetEntity& selectedTargetGUI);
	
	void handleEvent(const GameEvent& gameEvent, const Map& map) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler) override;
	void updateSelectionBox(TargetEntity& selectedTargetGUI);
	void render(ShaderHandler& shaderHandler) const override;
	void renderSelectionBox(const sf::Window& window) const;

private:
	PlannedBuilding m_plannedBuilding;
	SelectionBox m_selectionBox;
	glm::vec3 m_previousMouseToGroundPosition;
	bool m_attackMoveSelected;
	std::vector<Unit*> m_selectedUnits;
	
	void assignSelectedUnits();
	void instructWorkerReturnMinerals(const Map& map);
	void instructUnitToAttack(Unit& Unit, const Entity& targetEntity, eFactionController targetEntityOwningFaction, const Map& map);
	bool instructWorkerToBuild(const Map& map);

	void onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, TargetEntity& selectedTargetGUI);
	void onRightClick(const sf::Window& window, const Camera& camera,
		const std::vector<const Faction*>& opposingFactions, const Map& map);
	void onMouseMove(const sf::Window& window, const Camera& camera, const Map& map);

	template <class Entity>
	void selectEntity(std::list<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities = false,
		int keepEntityIDSelected = Globals::INVALID_ENTITY_ID)
	{
		auto selectedEntity = std::find_if(entities.begin(), entities.end(), [&mouseToGroundPosition](const auto& entity)
		{
			return entity.getAABB().contains(mouseToGroundPosition);
		});
		if (selectedEntity != entities.end())
		{
			if (selectAllEntities)
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
				if (keepEntityIDSelected == entity.getID())
				{
					entity.setSelected(true);
				}
				else
				{
					entity.setSelected(false);
				}
			}
		}
	}

	template <class Entity>
	void selectUnits(std::list<Entity>& units, const SelectionBox& selectionBox)
	{
		for (auto& unit : units)
		{
			unit.setSelected(m_selectionBox.getAABB().contains(unit.getAABB()));
		}
	}
};