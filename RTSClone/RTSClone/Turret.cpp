#include "Turret.h"
#include "ModelManager.h"
#include "FactionHandler.h"
#include "Globals.h"
#include "PathFinding.h"
#include "GameEventHandler.h"
#include "GameEvents.h"
#include "GameMessages.h"
#include "GameMessenger.h"

namespace
{
	const int TURRET_STARTING_HEALTH = 5;
	const int TURRET_DAMAGE = 2;
	const float TURRET_ATTACK_RANGE = Globals::NODE_SIZE * 7.0f;
	const float TIME_BETWEEN_ATTACK = 2.25f;
	const float TIME_BETWEEN_IDLE_CHECK = 1.0f;
}

Turret::Turret(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(TURRET_MODEL_NAME), startingPosition,
		eEntityType::Turret, TURRET_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_targetEntity(),
	m_currentState(eTurretState::Idle),
	m_stateHandlerTimer(0.2f, true),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{
	broadcastToMessenger<GameMessages::AddAABBToMap>({ m_AABB });
}

Turret::~Turret()
{
	if (m_status.isActive())
	{
		broadcastToMessenger<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

void Turret::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	Entity::update(deltaTime);
	m_stateHandlerTimer.update(deltaTime);
	m_attackTimer.update(deltaTime);

	switch (m_currentState)
	{
	case eTurretState::Idle:
	{
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.get().getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, TURRET_ATTACK_RANGE);
				if (targetEntity)
				{
					m_targetEntity.set(opposingFaction.get().getController(), targetEntity->getID(), targetEntity->getEntityType());
					switchToState(eTurretState::Attacking);
					break;
				}
			}
		}
	}
		break;
	case eTurretState::Attacking:
	{
		assert(m_targetEntity.getID() != Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& opposingFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID(), m_targetEntity.getType());
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map, m_AABB))
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position, 270.0f);
			}
			else
			{
				switchToState(eTurretState::Idle);
			}
		}

		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& opposingFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID(), m_targetEntity.getType());
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map, m_AABB))
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position, 270.0f);
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.get().getController(), getID(),
					getEntityType(), opposingFaction.getController(), targetEntity->getID(), targetEntity->getEntityType(), 
					TURRET_DAMAGE, m_position, targetEntity->getPosition()));
				
				m_attackTimer.resetElaspedTime();
			}
		}
	}
		break;
	default:
		assert(false);
	}

	if (m_stateHandlerTimer.isExpired())
	{
		m_stateHandlerTimer.resetElaspedTime();
	}
}

void Turret::switchToState(eTurretState newState)
{
	switch (newState)
	{
	case eTurretState::Idle:
		m_targetEntity.reset();
		break;
	case eTurretState::Attacking:
		m_attackTimer.resetElaspedTime();
		break;
	default:
		assert(false);
	}

	m_currentState = newState;
}