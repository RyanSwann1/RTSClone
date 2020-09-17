#include "Level.h"
#include "LevelFileHandler.h"
#include "GameEventHandler.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Camera.h"

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
	m_player(static_cast<FactionPlayer&>(*m_factions[static_cast<int>(eFactionController::Player)])),
	m_selectedTarget()
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

Level::~Level()
{
	GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>({});
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

	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed &&
		currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
	{
		glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
		const Entity* targetEntity = nullptr;
		eFactionController targetEntityFaction;
		for (const auto& faction : m_factions)
		{
			if (faction)
			{
				targetEntity = faction->getEntity(mouseToGroundPosition);
				if (targetEntity)
				{			
					targetEntityFaction = faction->getController();
					break;
				}
			}
		}

		if (!targetEntity)
		{
			m_selectedTarget.reset();
			GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>({});
		}
		else
		{
			m_selectedTarget.set(targetEntityFaction, targetEntity->getID());

			GameMessenger::getInstance().broadcast<GameMessages::UIDisplayEntity>(
				{ targetEntity->getHealth(), targetEntity->getEntityType() });
		}
	}
}

void Level::update(float deltaTime, const Map& map, bool& resetLevel)
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
	if (getActiveFactionCount(m_factions) == 1)
	{
		resetLevel = true;
	}

	if (m_factionHandler.isFactionActive(m_selectedTarget.getFactionController()))
	{
		const Entity* targetEntity = m_factionHandler.getFaction(m_selectedTarget.getFactionController()).getEntity(m_selectedTarget.getID());
		if (!targetEntity)
		{
			m_selectedTarget.reset();
			GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>({});
		}
		else
		{
			GameMessenger::getInstance().broadcast<GameMessages::UIDisplayEntity>(
				{ targetEntity->getHealth(), targetEntity->getEntityType() });
		}
	}
	else
	{
		m_selectedTarget.reset();
		GameMessenger::getInstance().broadcast<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>({});
	}
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