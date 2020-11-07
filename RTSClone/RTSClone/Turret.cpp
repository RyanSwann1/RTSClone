#include "Turret.h"
#include "ModelManager.h"
#include "FactionHandler.h"
#include "Globals.h"
#include "PathFinding.h"
#include "PathFindingLocator.h"
#include "GameEventHandler.h"
#include "GameEvent.h"

namespace
{
	constexpr int TURRET_STARTING_HEALTH = 5;
	constexpr int TURRET_DAMAGE = 2;
	constexpr float TURRET_ATTACK_RANGE = Globals::NODE_SIZE * 5.0f;
	constexpr float TIME_BETWEEN_ATTACK = 2.0f;
	constexpr float TIME_BETWEEN_IDLE_CHECK = 1.0f;
}

Turret::Turret(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(TURRET_MODEL_NAME), startingPosition,
		eEntityType::Turret, TURRET_STARTING_HEALTH),
	m_owningFaction(owningFaction),
	m_targetEntity(),
	m_currentState(eTurretState::Idle),
	m_idleTimer(TIME_BETWEEN_IDLE_CHECK, true),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{}

void Turret::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	m_idleTimer.update(deltaTime);
	m_attackTimer.update(deltaTime);

	switch (m_currentState)
	{
	case eTurretState::Idle:
	{
		if (!m_idleTimer.isExpired())
		{
			break;
		}

		for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
		{
			const Entity* targetEntity = opposingFaction->getEntity(m_position, TURRET_ATTACK_RANGE);
			if (targetEntity)
			{
				m_targetEntity.set(opposingFaction->getController(), targetEntity->getID());
				m_currentState = eTurretState::Attacking;
				break;
			}
		}
	}
		
		break;
	case eTurretState::Attacking:
	{
		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& opposingFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID());
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFindingLocator::get().isTargetInLineOfSight(m_position, *targetEntity, map))
			{
				GameEventHandler::getInstance().gameEvents.push({ eGameEventType::SpawnProjectile, m_owningFaction.getController(), getID(),
					opposingFaction.getController(), targetEntity->getID(), TURRET_DAMAGE, m_position, targetEntity->getPosition() });
			}
			else
			{
				m_currentState = eTurretState::Idle;
			}
		}
	}
		break;
	default:
		assert(false);
	}

	if (m_idleTimer.isExpired())
	{
		m_idleTimer.resetElaspedTime();
	}

	if (m_attackTimer.isExpired())
	{
		m_attackTimer.resetElaspedTime();
	}
}