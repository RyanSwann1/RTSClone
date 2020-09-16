#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"

Level::Level(std::vector<SceneryGameObject>&& scenery, 
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>&& factions)
	: m_scenery(std::move(scenery)),
	m_factions(std::move(factions)),
	m_factionHandler(m_factions),
	m_projectileHandler(),
	m_player(static_cast<FactionPlayer&>(*m_factions[static_cast<int>(eFactionController::Player)])) 
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

void Level::handleEvent(const GameEvent& gameEvent)
{
	switch (gameEvent.type)
	{
	case eGameEventType::FactionEliminated:
	{
		assert(m_factions[static_cast<int>(gameEvent.senderFaction)] &&
			m_factions[static_cast<int>(gameEvent.senderFaction)]->getController() == gameEvent.senderFaction);

		m_factions[static_cast<int>(gameEvent.senderFaction)].release();
		setAITargetFaction();
	}	
		break;
	default:
		assert(false);
	}
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map)
{
	m_player.handleInput(currentSFMLEvent, window, camera, map, m_factionHandler.getOpposingFactions(eFactionController::Player));
}

void Level::update(float deltaTime, const Map& map)
{
	m_projectileHandler.update(deltaTime, m_factionHandler);
	
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->update(deltaTime, map, m_factionHandler);
		}
	}

	GameEventHandler::getInstance().handleEvents(m_factionHandler, m_projectileHandler, map, *this);
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	m_player.renderSelectionBox(window);
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