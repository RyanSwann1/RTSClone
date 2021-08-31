#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "UIManager.h"
#include "Camera.h"
#include "AIConstants.h"
#include <imgui/imgui.h>

namespace
{
	constexpr float TIME_BETWEEN_UNIT_STATE = 0.05f;
	constexpr glm::vec3 TERRAIN_COLOR = { 0.9098039f, 0.5176471f, 0.3882353f };

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
	std::unique_ptr<BaseHandler>&& baseHandler, const glm::vec3& size, glm::vec2 cameraStartingPosition, 
	glm::ivec2 windowSize)
	: m_playableArea(size, TERRAIN_COLOR),
	m_baseHandler(std::move(baseHandler)),
	m_scenery(std::move(scenery)),
	m_camera(),
	m_minimap(),
	m_factions(std::move(factions)),
	m_unitStateHandlerTimer(TIME_BETWEEN_UNIT_STATE, true),
	m_factionHandler(m_factions),
	m_projectileHandler()
{
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			switch (faction->getController())
			{
				case eFactionController::Player:
				break;
				case eFactionController::AI_1:
				case eFactionController::AI_2:
				case eFactionController::AI_3:
					static_cast<FactionAI&>(*faction.get()).setTargetFaction(m_factionHandler);
				break;
				default:
				assert(false);
			}	
		}
	}

	m_camera.setPosition({ cameraStartingPosition.x, m_camera.position.y, cameraStartingPosition.y }, 
		m_playableArea.getSize(), windowSize, true);
	m_camera.maxDistanceFromGround = m_camera.position.y;
}

std::unique_ptr<Level> Level::load(const std::string& levelName, glm::ivec2 windowSize)
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
	glm::vec2 cameraStartingPosition(0.0f);
	static_assert(static_cast<int>(AIConstants::eBehaviour::Max) == 1, "Current assigning of AI behaviour relies on only two behaviours");
	int AIBehaviourIndex = 0;
	
	assert(factionCount < static_cast<int>(eFactionController::Max) + 1 && 
		factionCount <= static_cast<int>(baseHandler->getBases().size()));
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
			cameraStartingPosition = { headquartersPosition.z, headquartersPosition.x };
		}
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			factions[i] = std::make_unique<FactionAI>(eFactionController(i), baseHandler->getBases()[i].position,
				factionStartingResources, factionStartingPopulation, static_cast<AIConstants::eBehaviour>(AIBehaviourIndex), *baseHandler);
			AIBehaviourIndex ^= 1;
			break;
		default:
			assert(false);
		}
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery), std::move(factions), 
		std::move(baseHandler), size, cameraStartingPosition, windowSize));
}

const std::vector<SceneryGameObject>& Level::getSceneryGameObjects() const
{
	return m_scenery;
}

const BaseHandler& Level::getBaseHandler() const
{
	return *m_baseHandler.get();
}

const Camera& Level::getCamera() const
{
	return m_camera;
}

bool Level::isMinimapInteracted() const
{
	return m_minimap.isUserInteracted();
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
	broadcastToMessenger<GameMessages::UIClearSelectedMineral>({});
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

void Level::handleInput(glm::uvec2 windowSize, const sf::Window& window, const sf::Event& currentSFMLEvent, const Map& map,
	UIManager& uiManager)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
	{
		return;
	}

	m_minimap.handleInput(windowSize, window, getSize(), m_camera, currentSFMLEvent);
	if (m_minimap.isUserInteracted())
	{
		return;
	}

	if (isFactionActive(m_factions, eFactionController::Player))
	{
		getFactionPlayer(m_factions).handleInput(currentSFMLEvent, window, m_camera, map, m_factionHandler, 
			*m_baseHandler, m_minimap, getSize());
	}

	switch (currentSFMLEvent.type)
	{
		case sf::Event::MouseButtonPressed:
		{
			glm::vec3 position = m_camera.getRayToGroundPlaneIntersection(window);
			for (auto& faction : m_factions)
			{
				if (faction)
				{
					switch (faction.get()->getController())
					{
					case eFactionController::AI_1:
					case eFactionController::AI_2:
					case eFactionController::AI_3:
						static_cast<FactionAI&>(*faction).selectEntity(position);
						break;
					case eFactionController::Player:
						break;
					default:
						assert(false);
					}
				}
			}

			assert(m_baseHandler);
			bool mineralSelected = false;
			for (const auto& base : m_baseHandler->getBases())
			{
				for (const auto& mineral : base.getMinerals())
				{
					if (mineral.getAABB().contains(position))
					{
						mineralSelected = true;
						broadcastToMessenger<GameMessages::UIDisplaySelectedMineral>({ mineral.getQuantity() });
					}
				}
			}
			if (!mineralSelected)
			{
				broadcastToMessenger<GameMessages::UIClearSelectedMineral>({});
			}
		}
		break;
		case sf::Event::MouseWheelScrolled:
			m_camera.zoom(window, currentSFMLEvent.mouseWheelScroll.delta);
		break;
	}

	uiManager.handleInput(window, m_factionHandler, m_camera, currentSFMLEvent);
}

void Level::update(float deltaTime, const Map& map, UIManager& uiManager, glm::uvec2 windowSize, const sf::Window& window)
{
	if (!m_minimap.isUserInteracted())
	{
		m_camera.update(deltaTime, window, windowSize, m_playableArea.getSize());
	}
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
	int gameEventsSize = static_cast<int>(gameEvents.size());
	for (int i = 0; i < gameEventsSize; ++i)
	{
		const GameEvent& gameEvent = gameEvents.front();
		handleEvent(gameEvent, map);
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

void Level::renderEntityStatusBars(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const
{
	for (const auto& faction : m_factions)
	{
		if (faction)
		{
			faction->renderEntityStatusBars(shaderHandler, m_camera, windowSize);
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

void Level::renderMinimap(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const sf::Window& window) const
{
	m_minimap.render(shaderHandler, windowSize, *this, m_camera, window);
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
	case eGameEventType::AttachFactionToBase:
		m_baseHandler->handleEvent(gameEvent);
		getFaction(m_factions, gameEvent.data.attachFactionToBase.factionController).handleEvent(gameEvent, map, m_factionHandler);
		break;
	case eGameEventType::DetachFactionFromBase:
		getFaction(m_factions, gameEvent.data.detachFactionFromBase.factionController).handleEvent(gameEvent, map, m_factionHandler);
		m_baseHandler->handleEvent(gameEvent);
		break;
	case eGameEventType::ForceSelfDestructEntity:
		getFaction(m_factions, gameEvent.data.forceSelfDestructEntity.factionController).handleEvent(gameEvent, map, m_factionHandler);
		break;
	}
}