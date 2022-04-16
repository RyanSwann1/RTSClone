#include "Core/Level.h"
#include "Core/LevelFileHandler.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Graphics/ModelManager.h"
#include "UI/UIManager.h"
#include "Core/Camera.h"
#include "AI/AIConstants.h"
#include <imgui/imgui.h>

namespace
{
	constexpr glm::vec3 TERRAIN_COLOR = { 0.9098039f, 0.5176471f, 0.3882353f };
	std::queue<GameEvent> gameEvents = {};

	bool is_hit_entity(const Projectile& projectile, const FactionHandler& factionHandler)
	{
		const Entity* entity = nullptr;
		if (const Faction* targetFaction = factionHandler.getFaction(projectile.getSenderEvent().targetFaction))
		{
			entity =
				targetFaction->getEntity(projectile.getAABB(), projectile.getSenderEvent().targetID);
		}

		return entity;
	}
}

//Level
Level::Level(LevelDetailsFromFile&& levelDetails, glm::ivec2 windowSize)
	: m_baseHandler(std::move(levelDetails.bases)),
	m_scenery(std::move(levelDetails.scenery)),
	m_playableArea(levelDetails.size, TERRAIN_COLOR),
	m_map(m_scenery, m_baseHandler.getBases(), levelDetails.gridSize),
	m_factionHandler(m_baseHandler, levelDetails)
{
	for (auto& faction : m_factionHandler.getFactions())
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
	
	glm::vec2 cameraStartingPosition = {};
	if (FactionPlayer* player = m_factionHandler.getFactionPlayer())
	{
		glm::vec2 cameraStartingPosition(0.0f);
		if (const Headquarters* mainHeadquarters = player->getMainHeadquarters())
		{
			cameraStartingPosition = { mainHeadquarters->getPosition().z, mainHeadquarters->getPosition().x };
		}
	}
	else
	{
		for (const auto& faction : m_factionHandler.getFactions())
		{
			if (faction)
			{
				if (const Headquarters* mainHeadquarters = faction->getMainHeadquarters())
				{
					cameraStartingPosition = { mainHeadquarters->getPosition().z, mainHeadquarters->getPosition().x };
				}
			}
		}
	}

	m_camera.setPosition({ cameraStartingPosition.x, m_camera.position.y, cameraStartingPosition.y }, 
		m_playableArea.getSize(), windowSize, true);
	m_camera.maxDistanceFromGround = m_camera.position.y;
}

std::optional<LevelDetailsFromFile> Level::load(std::string_view levelName, glm::ivec2 windowSize)
{
	return LevelFileHandler::loadLevelFromFile(levelName);
}

void Level::add_event(const GameEvent& gameEvent)
{
	gameEvents.push(gameEvent);
}

const std::vector<SceneryGameObject>& Level::getSceneryGameObjects() const
{
	return m_scenery;
}

const BaseHandler& Level::getBaseHandler() const
{
	return m_baseHandler;
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
	return m_factionHandler.getFactions();
}

const glm::vec3& Level::getSize() const
{
	return m_playableArea.getSize();
}

Level::~Level()
{
	broadcast<GameMessages::UIClearDisplaySelectedEntity>({});
	broadcast<GameMessages::UIClearSelectedMineral>({});
}

const Faction* Level::getWinningFaction() const
{
	size_t factionCount = 
		std::count_if(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [](auto& faction) { return faction.get(); });
	if (factionCount == 1)
	{
		auto winningFaction = 
			std::find_if(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [](auto& faction) { return faction.get(); });
		assert(winningFaction != m_factionHandler.getFactions().cend());
		return winningFaction->get();
	}

	return nullptr;
}

void Level::handleInput(glm::uvec2 windowSize, const sf::Window& window, const sf::Event& currentSFMLEvent, UIManager& uiManager)
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

	if (FactionPlayer* factionPlayer = m_factionHandler.getFactionPlayer())
	{
		factionPlayer->handleInput(currentSFMLEvent, window, m_camera, m_map, m_factionHandler,
			m_baseHandler, m_minimap, getSize());
	}

	switch (currentSFMLEvent.type)
	{
		case sf::Event::MouseButtonPressed:
		{
			glm::vec3 position = m_camera.getRayToGroundPlaneIntersection(window);
			std::for_each(m_factionHandler.getFactions().begin(), m_factionHandler.getFactions().end(), [&position](auto& faction)
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
			});

			bool mineralSelected = false;
			for (const auto& base : m_baseHandler.getBases())
			{
				for (const auto& mineral : base.getMinerals())
				{
					if (mineral.getAABB().contains(position))
					{
						mineralSelected = true;
						broadcast<GameMessages::UIDisplaySelectedMineral>({ mineral });
					}
				}
			}
			if (!mineralSelected)
			{
				broadcast<GameMessages::UIClearSelectedMineral>({});
			}
		}
		break;
		case sf::Event::MouseWheelScrolled:
			m_camera.zoom(window, currentSFMLEvent.mouseWheelScroll.delta);
		break;
	}

	uiManager.handleInput(window, m_factionHandler, m_camera, currentSFMLEvent);
}

void Level::update(float deltaTime, UIManager& uiManager, glm::uvec2 windowSize, const sf::Window& window)
{
	if (!m_minimap.isUserInteracted())
	{
		m_camera.update(deltaTime, window, windowSize, m_playableArea.getSize());
	}

	for (auto& faction : m_factionHandler.getFactions())
	{
		if (faction)
		{
			faction->update(deltaTime, m_map, m_factionHandler, m_baseHandler);
		}
	}

	const auto now = std::chrono::high_resolution_clock::now();
	if (now - m_lastDelayedUpdate >= std::chrono::milliseconds(100))
	{
		for (auto& faction : m_factionHandler.getFactions())
		{
			if (faction)
			{
				faction->delayed_update(m_map, m_factionHandler);
			}
		}
		m_lastDelayedUpdate = now;
	}

	for (auto& projectile : m_projectiles)
	{
		projectile.update(deltaTime);
	}

	m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(), [this](auto& projectile)
	{
		bool hitEntity = is_hit_entity(projectile, this->m_factionHandler);
		if (hitEntity)
		{
			Level::add_event(GameEvent::create<TakeDamageEvent>({ projectile.getSenderEvent().senderFaction,
				projectile.getSenderEvent().senderID, projectile.getSenderEvent().senderEntityType, projectile.getSenderEvent().targetFaction,
				projectile.getSenderEvent().targetID, projectile.getSenderEvent().damage }));
		}
		return hitEntity || projectile.isReachedDestination();
	}), m_projectiles.end());

	const size_t gameEventsSize = gameEvents.size();
	for (size_t i = 0; i < gameEventsSize; ++i)
	{
		handleEvent(gameEvents.front(), m_map);
		uiManager.handleEvent(gameEvents.front());

		gameEvents.pop();
	}

	uiManager.update(m_factionHandler);
}

void Level::renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const
{
	if (const FactionPlayer* factionPlayer = m_factionHandler.getFactionPlayer())
	{
		factionPlayer->renderEntitySelector(window, shaderHandler);
	}
}

void Level::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
	std::for_each(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [&shaderHandler](auto& faction)
	{
		if(faction)
		{
			faction->renderPlannedBuildings(shaderHandler);
		}
	});
}

void Level::renderEntityStatusBars(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const
{
	std::for_each(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [&shaderHandler, windowSize, this](auto& faction)
	{
		if (faction)
		{
			faction->renderEntityStatusBars(shaderHandler, m_camera, windowSize);
		}
	});
}

void Level::renderTerrain(ShaderHandler& shaderHandler) const
{
	m_playableArea.render(shaderHandler);
}

void Level::renderPlayerPlannedBuilding(ShaderHandler& shaderHandler) const
{
	if (const FactionPlayer* factionPlayer = m_factionHandler.getFactionPlayer())
	{
		factionPlayer->renderPlannedBuilding(shaderHandler, m_baseHandler, m_map);
	}
}

void Level::renderBasePositions(ShaderHandler& shaderHandler) const
{
	m_baseHandler.renderBasePositions(shaderHandler);
}

void Level::renderMinimap(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const sf::Window& window) const
{
	m_minimap.render(shaderHandler, windowSize, *this, m_camera, window);
}

void Level::render(ShaderHandler& shaderHandler) const
{
	std::for_each(m_scenery.cbegin(), m_scenery.cend(), [&shaderHandler](auto& gameObject)
	{
		gameObject.render(shaderHandler);
	});

	std::for_each(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [&shaderHandler](auto& faction)
	{
		if (faction) { faction->render(shaderHandler); }
	});

	m_baseHandler.renderMinerals(shaderHandler);
	for (const auto& projectile : m_projectiles)
	{
		projectile.render(shaderHandler);
	}
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	std::for_each(m_factionHandler.getFactions().begin(), m_factionHandler.getFactions().end(), [&shaderHandler](auto& faction)
	{
		if (faction)
		{
			faction->renderAABB(shaderHandler);
		}
	});

	std::for_each(m_scenery.begin(), m_scenery.end(), [&shaderHandler](auto& faction)
	{
		sceneryGameObject.renderAABB(shaderHandler);
	});

	m_projectileHandler.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
void Level::renderPathing(ShaderHandler& shaderHandler)
{
	std::for_each(m_factionHandler.getFactions().cbegin(), m_factionHandler.getFactions().cend(), [&shaderHandler](auto& faction)
	{
		if (faction)
		{
			faction->renderPathing(shaderHandler);
		}
	});
}
#endif // RENDER_PATHING

void Level::handleEvent(const GameEvent& gameEvent, const Map& map)
{
	Faction* faction = { nullptr };
	switch (gameEvent.type)
	{
	case eGameEventType::IncreaseFactionShield:
		faction = m_factionHandler.getFaction(gameEvent.data.increaseFactionShield.factionController);
		break;
	case eGameEventType::TakeDamage:
		faction = m_factionHandler.getFaction(gameEvent.data.takeDamage.targetFaction);
		break;
	case eGameEventType::RepairEntity:
		faction = m_factionHandler.getFaction(gameEvent.data.repairEntity.factionController);
		break;
	case eGameEventType::PlayerActivatePlannedBuilding:
	case eGameEventType::PlayerSpawnEntity:
		if (FactionPlayer* factionPlayer = m_factionHandler.getFactionPlayer())
		{
			factionPlayer->handleEvent(gameEvent, map, m_factionHandler, m_baseHandler);
		}
		break;
	case eGameEventType::AttachFactionToBase:
		faction = m_factionHandler.getFaction(gameEvent.data.attachFactionToBase.factionController);
		m_baseHandler.handleEvent(gameEvent);
		break;
	case eGameEventType::DetachFactionFromBase:
		faction = m_factionHandler.getFaction(gameEvent.data.detachFactionFromBase.factionController);
		m_baseHandler.handleEvent(gameEvent);
		break;
	case eGameEventType::ForceSelfDestructEntity:
		faction = m_factionHandler.getFaction(gameEvent.data.forceSelfDestructEntity.factionController);
		break;
	case eGameEventType::EntityIdle:
		faction = m_factionHandler.getFaction(gameEvent.data.entityIdle.faction);
		break;
	case eGameEventType::AddFactionResources:
		faction = m_factionHandler.getFaction(gameEvent.data.addFactionResources.faction);
		break;
	}

	if (faction) 
	{
		faction->handleEvent(gameEvent, map, m_factionHandler, m_baseHandler);
	}

	switch (gameEvent.type)
	{
	case eGameEventType::RevalidateMovementPaths:
		std::for_each(m_factionHandler.getFactions().begin(), m_factionHandler.getFactions().end(), [&gameEvent, &map, this](auto& faction)
		{
			if (faction)
			{
				faction->handleEvent(gameEvent, map, m_factionHandler, m_baseHandler);
			}
		});
		break;
	case eGameEventType::HeadquartersDestroyed:
	{
		if (m_factionHandler.getFaction(gameEvent.data.headquartersDestroyed.factionController)->get_headquarters_count() == 0
			&& m_factionHandler.removeFaction(gameEvent.data.headquartersDestroyed.factionController))
		{
			for (auto& faction : m_factionHandler.getFactions())
			{
				if (faction && faction->getController() != eFactionController::Player)
				{
					static_cast<FactionAI&>(*faction).onFactionElimination(
						m_factionHandler, gameEvent.data.headquartersDestroyed.factionController);
				}
			}
		}
	}
		break;
	case eGameEventType::SpawnProjectile:
		m_projectiles.emplace_back(gameEvent.data.spawnProjectile);
		break;
	}
}