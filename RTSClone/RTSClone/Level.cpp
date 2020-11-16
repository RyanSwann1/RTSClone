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
			static_cast<FactionAI&>(*faction.get()).setTargetFaction(
				m_factionHandler.getOpposingFactions(faction->getController()));
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
			if (!Globals::UNIT_SPAWNER_TYPES.isMatch(targetEntity->getEntityType()))
			{
				switch (targetEntity->getEntityType())
				{
				case eEntityType::Unit:
				case eEntityType::SupplyDepot:
				case eEntityType::Turret:
					GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
						{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
						targetEntity->getHealth() });
					break;
				case eEntityType::Worker:
				{
					const Timer& buildTimer = static_cast<const Worker&>(*targetEntity).getBuildTimer();
					GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
						{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
						targetEntity->getHealth(), buildTimer.getExpiredTime() - buildTimer.getElaspedTime() });		
				}
					break;
				default:
					assert(false);
				}
			}
			else
			{
				const UnitSpawnerBuilding& unitSpawnerBuilding = static_cast<const UnitSpawnerBuilding&>(*targetEntity);
				GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
					{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
					unitSpawnerBuilding.getHealth(), unitSpawnerBuilding.getCurrentSpawnCount(), 
					unitSpawnerBuilding.getSpawnTimer().getExpiredTime() - unitSpawnerBuilding.getSpawnTimer().getElaspedTime() });
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

void Level::handleEvent(const GameEvent& gameEvent, const Map& map)
{
	switch (gameEvent.type)
	{
	case eGameEventType::TakeDamage:
		if (isFactionActive(m_factions, gameEvent.targetFaction))
		{
			getFaction(m_factions, gameEvent.targetFaction).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::RemovePlannedBuilding:
	case eGameEventType::RemoveAllWorkerPlannedBuildings:
	case eGameEventType::AddResources:
	case eGameEventType::RepairEntity:
		if (isFactionActive(m_factions, gameEvent.senderFaction))
		{
			getFaction(m_factions, gameEvent.senderFaction).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::SpawnProjectile:
		m_projectileHandler.addProjectile(gameEvent);
		break;
	case eGameEventType::RevalidateMovementPaths:
		for(auto& faction : m_factions)
		{
			if (faction)
			{
				faction->handleEvent(gameEvent, map, m_factionHandler);
			}
		}
		break;
	case eGameEventType::EliminateFaction:
		if (isFactionActive(m_factions, gameEvent.senderFaction))
		{
			m_factions[static_cast<int>(gameEvent.senderFaction)].reset();
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
	default:
		assert(false);
	}
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
	{
		return;
	}

	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed &&
		currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
	{
		glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
		const Entity* targetEntity = nullptr;
		for (const auto& faction : m_factions)
		{
			if (faction)
			{
				targetEntity = faction->getEntity(mouseToGroundPosition);
				if (targetEntity)
				{	
					m_selectedTargetGUI.set(faction->getController(), targetEntity->getID());
					break;
				}
			}
		}

		if(!targetEntity)
		{
			m_selectedTargetGUI.reset();
		}
	}

	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).handleInput(
		currentSFMLEvent, window, camera, map, m_factionHandler.getOpposingFactions(eFactionController::Player), m_selectedTargetGUI);
	}
}

void Level::update(float deltaTime, const Map& map)
{
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->update(deltaTime, map, m_factionHandler);
		}
	}
	
	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).updateSelectionBox(m_selectedTargetGUI);
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

void Level::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_scenery)
	{
		gameObject.render(shaderHandler);
	}

	for(auto& faction : m_factions)
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