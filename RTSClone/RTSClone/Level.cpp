#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "UIManager.h"
#include "Camera.h"
#include <imgui/imgui.h>

namespace
{
	const float TIME_BETWEEN_UNIT_STATE = 0.05f;

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

//Level
Level::Level(std::vector<SceneryGameObject>&& scenery, FactionsContainer&& factions, 
	std::vector<Base>&& mainBaseLocations)
	: m_mainBases(std::move(mainBaseLocations)),
	m_scenery(std::move(scenery)),
	m_factions(std::move(factions)),
	m_unitStateHandlerTimer(TIME_BETWEEN_UNIT_STATE, true),
	m_factionHandler(m_factions),
	m_projectileHandler()
{
	for (auto& faction : m_factions)
	{
		if (faction && faction->getController() != eFactionController::Player)
		{
			static_cast<FactionAI&>(*faction.get()).setTargetFaction(m_factionHandler);
		}
	}
}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	std::vector<Base> mainBases;
	std::vector<SceneryGameObject> scenery;
	int factionStartingResources = 0;
	int factionStartingPopulation = 0;
	int factionCount = 0;
	if (!LevelFileHandler::loadLevelFromFile(levelName, scenery, mainBases, 
		factionStartingResources, factionStartingPopulation, factionCount))
	{
		return std::unique_ptr<Level>();
	}

	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1> factions;
	assert(factionCount < static_cast<int>(eFactionController::Max) + 1 && factionCount < static_cast<int>(mainBases.size()));
	for (int i = 0; i < factionCount; ++i)
	{
		switch (eFactionController(i))
		{
		case eFactionController::Player:
			factions[i] = std::make_unique<FactionPlayer>(mainBases[i].position, 
				factionStartingResources, factionStartingPopulation);
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			factions[i] = std::make_unique<FactionAI>(eFactionController(i), mainBases[i].position,
				factionStartingResources, factionStartingPopulation, mainBases[i]);
			break;
		default:
			assert(false);
		}
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery), std::move(factions), std::move(mainBases)));
}

Level::~Level()
{
	broadcastToMessenger<GameMessages::UIClearDisplaySelectedEntity>({});
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

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map,
	UIManager& uiManager)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
	{
		return;
	}

	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).handleInput(currentSFMLEvent, window, camera, map, m_factionHandler, m_mainBases);
	}

	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
	{
		glm::vec3 planeIntersection = camera.getRayToGroundPlaneIntersection(window);
		for (auto& opposingFaction : m_factionHandler.getOpposingFactions(eFactionController::Player))
		{
			opposingFaction.get().selectEntity(planeIntersection);
		}
	}

	uiManager.handleInput(window, m_factionHandler, camera, currentSFMLEvent);
}

void Level::update(float deltaTime, const Map& map, UIManager& uiManager)
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

	m_projectileHandler.update(deltaTime, m_factionHandler);

	std::queue<GameEvent>& gameEvents = GameEventHandler::getInstance().gameEvents;
	while (!gameEvents.empty())
	{
		const GameEvent& gameEvent = gameEvents.front();
		handleEvent(gameEvent, map);
		uiManager.handleEvent(gameEvent);

		gameEvents.pop();
	}

	uiManager.update(m_factionHandler);
}

void Level::renderSelectionBox(const sf::Window& window, ShaderHandler& shaderHandler) const
{
	if (isFactionActive(m_factions, eFactionController::Player))
	{
		c_getFactionPlayer(m_factions).renderSelectionBox(window, shaderHandler);
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

	for (const auto& base : m_mainBases)
	{
		for (const auto& mineral : base.minerals)
		{
			mineral.render(shaderHandler);
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
			for (auto& faction : m_factions)
			{
				if (faction && faction->getController() != eFactionController::Player)
				{
					static_cast<FactionAI&>(*faction).onFactionElimination(
						m_factionHandler, gameEvent.data.eliminateFaction.factionController);
				}
			}
		}
		break;
	case eGameEventType::PlayerActivatePlannedBuilding:
	case eGameEventType::PlayerSpawnUnit:
		if (isFactionActive(m_factions, eFactionController::Player))
		{
			getFaction(m_factions, eFactionController::Player).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	}
}