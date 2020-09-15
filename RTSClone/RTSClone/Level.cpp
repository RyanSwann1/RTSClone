#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"

namespace
{
	Faction& getFaction(std::vector<std::unique_ptr<Faction>>& factions, eFactionController factionController)
	{
		auto faction = std::find_if(factions.begin(), factions.end(), [factionController](const auto& faction)
		{
			return faction->getController() == factionController;
		});
		assert(faction != factions.end());
		
		return *faction->get();
	}
}

Level::Level(std::vector<SceneryGameObject>&& scenery, std::vector<std::unique_ptr<Faction>>&& factions)
	: m_scenery(std::move(scenery)),
	m_factions(std::move(factions)),
	m_factionHandler(m_factions),
	m_projectileHandler(),
	m_player(static_cast<FactionPlayer&>(getFaction(m_factions, eFactionController::Player)))
{}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	std::vector<SceneryGameObject> scenery;
	std::vector<std::unique_ptr<Faction>> factions;
	if (!LevelFileHandler::loadLevelFromFile(levelName, scenery, factions))
	{
		return std::unique_ptr<Level>();
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery), std::move(factions)));
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map)
{
	m_player.handleInput(currentSFMLEvent, window, camera, map, m_factionHandler.getOpposingFactions(eFactionController::Player));
	// getOpposingFactions(m_player->getController()));
}

void Level::update(float deltaTime, const Map& map)
{
	m_projectileHandler.update(deltaTime, m_factionHandler);
	
	for (auto& faction : m_factions)
	{
		faction->update(deltaTime, map, m_factionHandler);
	}

	GameEventHandler::getInstance().handleEvents(m_factionHandler, m_projectileHandler, map);
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	m_player.renderSelectionBox(window);
}

void Level::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
	for (auto& faction : m_factions)
	{
		faction->renderPlannedBuildings(shaderHandler);
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
		faction->render(shaderHandler);
	}

	m_projectileHandler.render(shaderHandler);
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	for (auto& faction : m_factions)
	{
		faction->renderAABB(shaderHandler);
	}

	m_projectileHandler.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
void Level::renderPathing(ShaderHandler& shaderHandler)
{
	for (auto& faction : m_factions)
	{
		faction->renderPathing(shaderHandler);
	}
}
#endif // RENDER_PATHING