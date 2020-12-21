#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Camera.h"
#include <imgui/imgui.h>

namespace
{
	const float TIME_BETWEEN_UNIT_STATE = 0.2f;

	int getActiveFactionCount(const FactionsContainer& factions)
	{
		int activeFactions = 0;
		for (const auto& faction : factions)
		{
			if (faction)
			{
				++activeFactions;
			}
		}

		assert(activeFactions > 0);
		return activeFactions;
	}

	bool isFactionActive(const FactionsContainer& factions, eFactionController factionController)
	{
		return factions[static_cast<int>(factionController)].get();
	}

	FactionPlayer& getFactionPlayer(FactionsContainer& factions)
	{
		assert(isFactionActive(factions, eFactionController::Player));
		return static_cast<FactionPlayer&>(*factions[static_cast<int>(eFactionController::Player)]);
	}

	const FactionPlayer& c_getFactionPlayer(const FactionsContainer& factions)
	{
		assert(isFactionActive(factions, eFactionController::Player));
		return static_cast<FactionPlayer&>(*factions[static_cast<int>(eFactionController::Player)]);
	}

	Faction& getFaction(FactionsContainer& factions, eFactionController factionController)
	{
		assert(isFactionActive(factions, factionController));
		return *factions[static_cast<int>(factionController)].get();
	}
}

Level::Level(std::vector<SceneryGameObject>&& scenery, FactionsContainer&& factions)
	: m_scenery(std::move(scenery)),
	m_factions(std::move(factions)),
	m_unitStateHandlerTimer(TIME_BETWEEN_UNIT_STATE, true),
	m_factionHandler(m_factions),
	m_projectileHandler(),
	m_selectedTargetGUI()
{
	setAITargetFaction();
}

void Level::setAITargetFaction()
{
	for (auto& faction : m_factions)
	{
		if (faction && faction->getController() != eFactionController::Player)
		{
			static_cast<FactionAI&>(*faction.get()).setTargetFaction(m_factionHandler);
		}
	}
}

void Level::handleGUI()
{
	if (m_selectedTargetGUI.getID() != Globals::INVALID_ENTITY_ID &&
		m_factionHandler.isFactionActive(m_selectedTargetGUI.getFactionController()))
	{
		const Entity* targetEntity = m_factionHandler.getFaction(m_selectedTargetGUI.getFactionController()).getEntity(m_selectedTargetGUI.getID());
		if (!targetEntity)
		{
			m_selectedTargetGUI.reset();
			GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>>({});
		}
		else
		{
			switch (targetEntity->getEntityType())
			{
			case eEntityType::HQ:
			case eEntityType::Barracks:
			{
				const UnitSpawnerBuilding& unitSpawnerBuilding = static_cast<const UnitSpawnerBuilding&>(*targetEntity);

				GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
					{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
					unitSpawnerBuilding.getHealth(), unitSpawnerBuilding.getShield(), unitSpawnerBuilding.getCurrentSpawnCount(),
					unitSpawnerBuilding.getSpawnTimer().getExpiredTime() - unitSpawnerBuilding.getSpawnTimer().getElaspedTime() });
			}
				break;
			case eEntityType::Unit:
			case eEntityType::SupplyDepot:
			case eEntityType::Turret:
			case eEntityType::Laboratory:
				GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
					{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth(), targetEntity->getShield() });
				break;
			case eEntityType::Worker:
			{
				const Timer& buildTimer = static_cast<const Worker&>(*targetEntity).getBuildTimer();
				GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
					{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth(), targetEntity->getShield(), buildTimer.getExpiredTime() - buildTimer.getElaspedTime() });
			}
			break;
			default:
				assert(false);
			}
		}
	}
	else
	{
		m_selectedTargetGUI.reset();
		GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>>({});
	}
}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	std::vector<SceneryGameObject> scenery;
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1> factions;
	if (!LevelFileHandler::loadLevelFromFile(levelName, scenery, factions))
	{
		return std::unique_ptr<Level>();
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery), std::move(factions)));
}

Level::~Level()
{
	GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>>({});
}

const Faction* Level::getPlayer() const
{
	if (m_factionHandler.isFactionActive(eFactionController::Player))
	{
		return &m_factionHandler.getFaction(eFactionController::Player);
	}

	return nullptr;
}

const Faction* Level::getWinningFaction() const
{
	const Faction* winningFaction = nullptr;
	if (getActiveFactionCount(m_factions) == 1)
	{
		for (const auto& faction : m_factions)
		{
			if (faction)
			{
				winningFaction = faction.get(); 
				break;
			}
		}
	}

	return winningFaction;
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
	{
		return;
	}

	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
	{
		glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
		if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
		{
			const Entity* selectedEntity = nullptr;
			for (const auto& faction : m_factions)
			{
				if (faction)
				{
					selectedEntity = faction->getEntity(mouseToGroundPosition);
					if (selectedEntity)
					{
						m_selectedTargetGUI.set(faction->getController(), selectedEntity->getID());
						break;
					}
				}
			}

			if (!selectedEntity)
			{
				m_selectedTargetGUI.reset();
			}
		}

		for (auto& opposingFaction : m_factionHandler.getOpposingFactions(eFactionController::Player))
		{
			opposingFaction.get().selectEntity(mouseToGroundPosition);
		}
	}

	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).handleInput(currentSFMLEvent, window, camera, map, m_factionHandler);
	}
}

void Level::update(float deltaTime, const Map& map)
{
	m_unitStateHandlerTimer.update(deltaTime);

	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->update(deltaTime, map, m_factionHandler, m_unitStateHandlerTimer);
		}
	}
	
	if (m_unitStateHandlerTimer.isExpired())
	{
		m_unitStateHandlerTimer.resetElaspedTime();
	}

	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).updateSelectionBox();
	}

	m_projectileHandler.update(deltaTime, m_factionHandler);

	std::queue<GameEvent>& gameEvents = GameEventHandler::getInstance().gameEvents;
	while (!gameEvents.empty())
	{
		const GameEvent& gameEvent = gameEvents.front();
		handleEvent(gameEvent, map);

		gameEvents.pop();
	}

	handleGUI();
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	if (isFactionActive(m_factions, eFactionController::Player))
	{
		c_getFactionPlayer(m_factions).renderSelectionBox(window);
	}
}

void Level::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->renderPlannedBuildings(shaderHandler);
		}
	}
}

void Level::renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	for (const auto& faction : m_factions)
	{
		if (faction)
		{
			faction->renderEntityStatusBars(shaderHandler, camera, windowSize);
		}
	}
}

void Level::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_scenery)
	{
		gameObject.render(shaderHandler);
	}

	for(const auto& faction : m_factions)
	{
		if (faction)
		{
			faction->render(shaderHandler);
		}
	}

	m_projectileHandler.render(shaderHandler);
	ModelManager::getInstance().getModel(TERRAIN_MODEL_NAME).render(shaderHandler, Globals::TERRAIN_POSITION);
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->renderAABB(shaderHandler);
		}
	}

	for (auto& sceneryGameObject : m_scenery)
	{
		sceneryGameObject.renderAABB(shaderHandler);
	}

	m_projectileHandler.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
void Level::renderPathing(ShaderHandler& shaderHandler)
{
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->renderPathing(shaderHandler);
		}
	}
}
#endif // RENDER_PATHING

void Level::handleEvent(const GameEvent& gameEvent, const Map& map)
{
	switch (gameEvent.type)
	{
	case eGameEventType::IncreaseFactionShield:
		if (isFactionActive(m_factions, gameEvent.data.increaseFactionShield.factionController))
		{
			getFaction(m_factions, gameEvent.data.increaseFactionShield.factionController).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::TakeDamage:
		if (isFactionActive(m_factions, gameEvent.data.takeDamage.targetFaction))
		{
			getFaction(m_factions, gameEvent.data.takeDamage.targetFaction).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::RemovePlannedBuilding:
		if (isFactionActive(m_factions, gameEvent.data.removePlannedBuilding.factionController))
		{
			getFaction(m_factions, gameEvent.data.removePlannedBuilding.factionController).
				handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::RemoveAllWorkerPlannedBuildings:
		if (isFactionActive(m_factions, gameEvent.data.removeAllWorkerPlannedBuilding.factionController))
		{
			getFaction(m_factions, gameEvent.data.removeAllWorkerPlannedBuilding.factionController).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::AddResources:
		if (isFactionActive(m_factions, gameEvent.data.addResources.factionController))
		{
			getFaction(m_factions, gameEvent.data.addResources.factionController).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::RepairEntity:
		if (isFactionActive(m_factions, gameEvent.data.repairEntity.factionController))
		{
			getFaction(m_factions, gameEvent.data.repairEntity.factionController).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::SpawnProjectile:
		m_projectileHandler.addProjectile(gameEvent);
		break;
	case eGameEventType::RevalidateMovementPaths:
		for (auto& faction : m_factions)
		{
			if (faction)
			{
				faction->handleEvent(gameEvent, map, m_factionHandler);
			}
		}
		break;
	case eGameEventType::EliminateFaction:
		if (isFactionActive(m_factions, gameEvent.data.eliminateFaction.factionController))
		{
			m_factions[static_cast<int>(gameEvent.data.eliminateFaction.factionController)].reset();
			setAITargetFaction();
		}
		break;
	case eGameEventType::PlayerActivatePlannedBuilding:
	case eGameEventType::PlayerSpawnUnit:
		if (isFactionActive(m_factions, eFactionController::Player))
		{
			getFaction(m_factions, eFactionController::Player).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::SetTargetEntityGUI:
		m_selectedTargetGUI.set(gameEvent.data.setTargetEntityGUI.factionController, 
			gameEvent.data.setTargetEntityGUI.entityID);
		break;
	case eGameEventType::ResetTargetEntityGUI:
		m_selectedTargetGUI.reset();
		break;
	default:
		assert(false);
	}
}