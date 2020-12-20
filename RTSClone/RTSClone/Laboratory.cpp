#include "Laboratory.h"
#include "ModelManager.h"
#include "Globals.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Faction.h"
#include "ShaderHandler.h"
#include "Camera.h"

namespace
{
	const float INCREASE_SHIELD_TIMER_EXPIRATION = 5.0f;
	const float PROGRESS_BAR_YOFFSET = 125.0f;
}

Laboratory::Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME), startingPosition, eEntityType::Laboratory,
		Globals::LABORATORY_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_increaseShieldCommands(),
	m_increaseShieldTimer(INCREASE_SHIELD_TIMER_EXPIRATION, false)
{
	GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB });
}

Laboratory::~Laboratory()
{
	GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB });
}

void Laboratory::addIncreaseShieldCommand(const std::function<void()>& command)
{
	if (m_increaseShieldCommands.empty())
	{
		m_increaseShieldTimer.resetElaspedTime();
		m_increaseShieldTimer.setActive(true);
	}
	
	m_increaseShieldCommands.push(command);
}

void Laboratory::update(float deltaTime)
{
	Entity::updateShieldReplenishTimer(deltaTime);

	if (m_increaseShieldTimer.isActive())
	{
		assert(!m_increaseShieldCommands.empty());

		m_increaseShieldTimer.update(deltaTime);
		if (m_increaseShieldTimer.isExpired())
		{
			m_increaseShieldCommands.front()(); //Function pointer
			m_increaseShieldCommands.pop();

			m_increaseShieldTimer.setActive(!m_increaseShieldCommands.empty());
			m_increaseShieldTimer.resetElaspedTime();
		}
	}
}

void Laboratory::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_increaseShieldTimer.isActive())
	{
		assert(!m_increaseShieldCommands.empty());

		float currentTime = m_increaseShieldTimer.getElaspedTime() / m_increaseShieldTimer.getExpiredTime();
		m_statbarSprite.render(m_position, windowSize, Globals::LABORATORY_STAT_BAR_WIDTH * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT,
			PROGRESS_BAR_YOFFSET, shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}