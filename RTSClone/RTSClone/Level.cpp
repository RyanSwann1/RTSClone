#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"

namespace
{
	Faction& getFaction(std::vector<std::unique_ptr<Faction>>& factions, eFactionName factionName)
	{
		auto faction = std::find_if(factions.begin(), factions.end(), [factionName](const auto& faction)
		{
			return faction->getName() == factionName;
		});
		assert(faction != factions.end());
		
		return *faction->get();
	}
}

Level::Level(std::vector<SceneryGameObject>&& scenery, std::vector<std::unique_ptr<Faction>>&& factions)
	: m_scenery(std::move(scenery)),
	m_projectileHandler(),
	m_factions(std::move(factions)),
	m_player(static_cast<FactionPlayer*>(&getFaction(m_factions, eFactionName::Player))),
	m_playerAI(static_cast<FactionAI*>(&getFaction(m_factions, eFactionName::AI)))
{
	for (const auto& gameObject : m_scenery)
	{
		if (gameObject.modelName != eModelName::Terrain)
		{
			AABB AABB(gameObject.position, ModelManager::getInstance().getModel(gameObject.modelName));
			GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ AABB });
		}
	}
}

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

Level::~Level()
{
	for (auto gameObject = m_scenery.begin(); gameObject != m_scenery.end();)
	{
		if (gameObject->modelName != eModelName::Terrain)
		{
			AABB AABB(gameObject->position, ModelManager::getInstance().getModel(gameObject->modelName));
			GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ AABB });
		}

		gameObject = m_scenery.erase(gameObject);
	}
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map)
{
	m_player->handleInput(currentSFMLEvent, window, camera, map, *m_playerAI);
}

void Level::update(float deltaTime, const Map& map)
{
	m_projectileHandler.update(deltaTime, *m_player, *m_playerAI);
	
	m_player->update(deltaTime, map, *m_playerAI);
	m_playerAI->update(deltaTime, map, *m_player);

	GameEventHandler::getInstance().handleEvents(*m_player, *m_playerAI, m_projectileHandler, map);
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	m_player->renderSelectionBox(window);
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