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
	int getActiveFactionCount(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions)
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
}

Level::Level(std::vector<SceneryGameObject>&& scenery, 
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>&& factions)
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
				GameMessenger::getInstance().broadcast<GameMessages::UIDisplaySelectedEntity>(
					{ m_selectedTargetGUI.getFactionController(), m_selectedTargetGUI.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth() });
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

eFactionController Level::getWinningFactionController() const
{
	assert(isComplete());
	eFactionController winningFaction;
	for (const auto& faction : m_factions)
	{
		if (faction)
		{
			winningFaction = faction->getController();
			break;
		}
	}

	return winningFaction;
}

bool Level::isComplete() const
{
	assert(getActiveFactionCount(m_factions) >= 1);
	return getActiveFactionCount(m_factions) == 1;
}

void Level::handleEvent(const GameEvent& gameEvent, const Map& map)
{
	switch (gameEvent.type)
	{
	case eGameEventType::Attack:
		if (m_factions[static_cast<int>(gameEvent.targetFaction)])
		{
			m_factions[static_cast<int>(gameEvent.targetFaction)]->handleEvent(gameEvent, map);
		}
		break;
	case eGameEventType::RemovePlannedBuilding:
	case eGameEventType::RemoveAllWorkerPlannedBuildings:
	case eGameEventType::AddResources:
		if (m_factions[static_cast<int>(gameEvent.senderFaction)])
		{
			m_factions[static_cast<int>(gameEvent.senderFaction)]->handleEvent(gameEvent, map);
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
				faction->handleEvent(gameEvent, map);
			}
		}
		break;
	case eGameEventType::FactionEliminated:
	{
		assert(m_factions[static_cast<int>(gameEvent.senderFaction)] &&
			m_factions[static_cast<int>(gameEvent.senderFaction)]->getController() == gameEvent.senderFaction);

		m_factions[static_cast<int>(gameEvent.senderFaction)].reset();
		setAITargetFaction();
	}	
		break;
	case eGameEventType::SpawnUnit:
		if (m_factions[static_cast<int>(gameEvent.targetFaction)])
		{
			m_factions[static_cast<int>(gameEvent.targetFaction)]->handleEvent(gameEvent, map);
		}
		break;
	case eGameEventType::ActivatePlayerPlannedBuilding:
		if (m_factions[static_cast<int>(gameEvent.targetFaction)])
		{
			m_factions[static_cast<int>(gameEvent.targetFaction)]->handleEvent(gameEvent, map);
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

	if (m_factions[static_cast<int>(eFactionController::Player)])
	{
		static_cast<FactionPlayer&>(*m_factions[static_cast<int>(eFactionController::Player)]).handleInput(
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
	
	m_projectileHandler.update(deltaTime, m_factionHandler);
	GameEventHandler::getInstance().handleEvents(*this, map);
	handleGUI();
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	if (m_factions[static_cast<int>(eFactionController::Player)])
	{
		static_cast<FactionPlayer&>(*m_factions[static_cast<int>(eFactionController::Player)]).renderSelectionBox(window);
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