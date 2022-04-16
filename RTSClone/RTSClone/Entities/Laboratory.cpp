#include "Laboratory.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Factions/Faction.h"
#include "Graphics/ShaderHandler.h"
#include "Core/Camera.h"
#include "Events/GameEvents.h"
#include "Core/Level.h"

namespace
{
	const float INCREASE_SHIELD_TIMER_EXPIRATION = 5.0f;
	const float PROGRESS_BAR_YOFFSET = 125.0f;
}

Laboratory::Laboratory(const glm::vec3& startingPosition, Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME), startingPosition, eEntityType::Laboratory,
		Globals::LABORATORY_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_shieldUpgradeCounter(0),
	m_increaseShieldTimer(INCREASE_SHIELD_TIMER_EXPIRATION, false)
{
	broadcast<GameMessages::AddAABBToMap>({ m_AABB });
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

Laboratory::~Laboratory()
{
	if (getID() != INVALID_ENTITY_ID)
	{
		broadcast<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

int Laboratory::getShieldUpgradeCounter() const
{
	return m_shieldUpgradeCounter;
}

void Laboratory::handleEvent(IncreaseFactionShieldEvent gameEvent)
{
	assert(m_owningFaction.get().getCurrentShieldAmount() < Globals::MAX_FACTION_SHIELD_AMOUNT &&
		m_owningFaction.get().isAffordable(Globals::FACTION_SHIELD_INCREASE_COST));

	if (m_shieldUpgradeCounter == 0)
	{
		m_increaseShieldTimer.resetElaspedTime();
		m_increaseShieldTimer.setActive(true);
	}

	++m_shieldUpgradeCounter;
}

void Laboratory::update(float deltaTime)
{
	Entity::update(deltaTime);

	if (m_increaseShieldTimer.isActive())
	{
		assert(m_shieldUpgradeCounter > 0);

		m_increaseShieldTimer.update(deltaTime);
		if (m_increaseShieldTimer.isExpired())
		{
			if (m_owningFaction.get().increaseShield(*this))
			{
				--m_shieldUpgradeCounter;
			}
			else
			{
				m_shieldUpgradeCounter = 0;
			}

			m_increaseShieldTimer.setActive(m_shieldUpgradeCounter > 0);
			m_increaseShieldTimer.resetElaspedTime();
		}
	}
}

void Laboratory::render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	Entity::render_status_bars(shaderHandler, camera, windowSize);
	if (m_increaseShieldTimer.isActive())
	{
		assert(m_shieldUpgradeCounter > 0);

		float currentTime = m_increaseShieldTimer.getElaspedTime() / m_increaseShieldTimer.getExpiredTime();
		m_statbarSprite.render(m_position, windowSize, Globals::LABORATORY_STAT_BAR_WIDTH,
			Globals::LABORATORY_STAT_BAR_WIDTH * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT,
			PROGRESS_BAR_YOFFSET, shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}