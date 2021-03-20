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
	const glm::vec3 TERRAIN_COLOR = { 0.9098039f, 0.5176471f, 0.3882353f };

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

	FactionPlayer& getFactionPlayer(const FactionsContainer& factions)
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
	std::unique_ptr<BaseHandler>&& baseHandler, const glm::vec3& size)
	: m_playableArea(size, TERRAIN_COLOR),
	m_baseHandler(std::move(baseHandler)),
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

std::unique_ptr<Level> Level::create(const std::string& levelName, Camera& camera)
{
	std::vector<Base> mainBases;
	std::vector<SceneryGameObject> scenery;
	int factionStartingResources = 0;
	int factionStartingPopulation = 0;
	int factionCount = 0;
	glm::vec3 size(0.0f);
	if (!LevelFileHandler::loadLevelFromFile(levelName, scenery, mainBases, 
		factionStartingResources, factionStartingPopulation, factionCount, size))
	{
		return std::unique_ptr<Level>();
	}

	std::unique_ptr<BaseHandler> baseHandler = std::make_unique<BaseHandler>(std::move(mainBases));
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1> factions;
	
	assert(factionCount < static_cast<int>(eFactionController::Max) + 1 && 
		factionCount < static_cast<int>(baseHandler->getBases().size()));
	for (int i = 0; i < factionCount; ++i)
	{
		assert(!factions[i]);
		switch (eFactionController(i))
		{
		case eFactionController::Player:
		{
			factions[i] = std::make_unique<FactionPlayer>(baseHandler->getBases()[i].position,
				factionStartingResources, factionStartingPopulation);

			const glm::vec3& headquartersPosition = factions[i]->getMainHeadquartersPosition();
			camera.setPosition({ headquartersPosition.z, headquartersPosition.x });
		}
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			factions[i] = std::make_unique<FactionAI>(eFactionController(i), baseHandler->getBases()[i].position,
				factionStartingResources, factionStartingPopulation, *baseHandler);
			break;
		default:
			assert(false);
		}
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery), std::move(factions), std::move(baseHandler), size));
}

const FactionsContainer& Level::getFactions() const
{
	return m_factions;
}

const glm::vec3& Level::getSize() const
{
	return m_playableArea.getSize();
}

Level::~Level()
{
	broadcastToMessenger<GameMessages::UIClearDisplaySelectedEntity>({});
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
		getFactionPlayer(m_factions).handleInput(currentSFMLEvent, window, camera, map, m_factionHandler, *m_baseHandler);
	}

	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
	{
		for (auto& faction : m_factions)
		{
			if (faction)
			{
				switch (faction.get()->getController())
				{
				case eFactionController::AI_1:
				case eFactionController::AI_2:					
				case eFactionController::AI_3:
					static_cast<FactionAI&>(*faction).selectEntity(camera.getRayToGroundPlaneIntersection(window));
					break;
				case eFactionController::Player:
					break;
				default:
					assert(false);
				}
			}
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
	for (int i = 0; i < gameEvents.size(); ++i)
	{
		const GameEvent& gameEvent = gameEvents.front();
		handleEvent(gameEvent, map);
		m_baseHandler->handleEvent(gameEvent);
		uiManager.handleEvent(gameEvent);

		gameEvents.pop();
	}

	uiManager.update(m_factionHandler);
}

void Level::renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const
{
	if (isFactionActive(m_factions, eFactionController::Player))
	{
		c_getFactionPlayer(m_factions).renderEntitySelector(window, shaderHandler);
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

void Level::renderTerrain(ShaderHandler& shaderHandler) const
{
	m_playableArea.render(shaderHandler);
}

void Level::renderPlayerPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const
{
	if (m_factionHandler.isFactionActive(eFactionController::Player))
	{
		getFactionPlayer(m_factions).renderPlannedBuilding(shaderHandler, *m_baseHandler, map);
	}
}

void Level::renderBasePositions(ShaderHandler& shaderHandler) const
{
	m_baseHandler->renderBasePositions(shaderHandler);
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

	m_baseHandler->renderMinerals(shaderHandler);
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
	case eGameEventType::PlayerSpawnEntity:
		if (isFactionActive(m_factions, eFactionController::Player))
		{
			getFaction(m_factions, eFactionController::Player).handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	case eGameEventType::OnEnteredIdleState:
		if (isFactionActive(m_factions, gameEvent.data.onEnteredIdleState.factionController))
		{
			m_factions[static_cast<int>(gameEvent.data.onEnteredIdleState.factionController)]->
				handleEvent(gameEvent, map, m_factionHandler);
		}
		break;
	}
}