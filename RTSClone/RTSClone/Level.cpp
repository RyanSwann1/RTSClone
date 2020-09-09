#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"

Level::Level(std::vector<GameObject>&& scenery)
	: m_scenery(std::move(scenery)),
	m_map(),
	m_projectileHandler(),
	m_player(eFactionName::Player, { 35.0f, Globals::GROUND_HEIGHT, 15.f }, { 70.0f, Globals::GROUND_HEIGHT, Globals::NODE_SIZE }),
	m_playerAI(eFactionName::AI, { 35.0f, Globals::GROUND_HEIGHT, 100.0f }, { 70.0f, Globals::GROUND_HEIGHT, 100.0f }, m_player)
{
	for (const auto& gameObject : m_scenery)
	{
		if (gameObject.modelName != eModelName::Terrain)
		{
			GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ gameObject.AABB });
		}
	}
}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	std::vector<GameObject> scenery;
	if (!LevelFileHandler::loadLevelFromFile(levelName, scenery))
	{
		return std::unique_ptr<Level>();
	}

	return std::unique_ptr<Level>(new Level(std::move(scenery)));
}

Level::~Level()
{
	for (const auto& gameObject : m_scenery)
	{
		if (gameObject.modelName != eModelName::Terrain)
		{
			GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ gameObject.AABB });
		}
	}
}

void Level::handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent)
{
	m_player.handleInput(currentSFMLEvent, window, camera, m_map, m_playerAI);
}

void Level::update(float deltaTime)
{
	m_projectileHandler.update(deltaTime, m_player, m_playerAI);
	m_player.update(deltaTime, m_map, m_playerAI);
	m_playerAI.update(deltaTime, m_map, m_player);

	GameEventHandler::getInstance().handleEvents(m_player, m_playerAI, m_projectileHandler, m_map);
}

void Level::renderSelectionBox(const sf::Window& window) const
{
	m_player.renderSelectionBox(window);
}

void Level::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
	m_player.renderPlannedBuildings(shaderHandler);
	m_playerAI.renderPlannedBuildings(shaderHandler);
}

void Level::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_scenery)
	{
		gameObject.render(shaderHandler);
	}

	m_player.render(shaderHandler);
	m_playerAI.render(shaderHandler);
	m_projectileHandler.render(shaderHandler);
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	m_player.renderAABB(shaderHandler);
	m_playerAI.renderAABB(shaderHandler);
	m_projectileHandler.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
void Level::renderPathing(ShaderHandler& shaderHandler)
{
	m_player.renderPathing(shaderHandler);
	m_playerAI.renderPathing(shaderHandler);
}
#endif // RENDER_PATHING