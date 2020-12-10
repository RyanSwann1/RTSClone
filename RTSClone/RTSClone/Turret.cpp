#include "Turret.h"
#include "ModelManager.h"
#include "FactionHandler.h"
#include "Globals.h"
#include "PathFinding.h"
#include "GameEventHandler.h"
#include "GameEvent.h"
#include "GameMessages.h"
#include "GameMessenger.h"

namespace
{
	constexpr int TURRET_STARTING_HEALTH = 5;
	constexpr int TURRET_DAMAGE = 2;
	constexpr float TURRET_ATTACK_RANGE = Globals::NODE_SIZE * 7.0f;
	constexpr float TIME_BETWEEN_ATTACK = 2.25f;
	constexpr float TIME_BETWEEN_IDLE_CHECK = 1.0f;
}

Turret::Turret(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(TURRET_MODEL_NAME), startingPosition,
		eEntityType::Turret, TURRET_STARTING_HEALTH),
	m_owningFaction(owningFaction),
	m_targetEntity(),
	m_currentState(eTurretState::Idle),
	m_stateHandlerTimer(0.2f, true),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{
	GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
}

Turret::~Turret()
{
	GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB, getID() });
}

void Turret::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	m_stateHandlerTimer.update(deltaTime);
	m_attackTimer.update(deltaTime);

	switch (m_currentState)
	{
	case eTurretState::Idle:
	{
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, TURRET_ATTACK_RANGE);
				if (targetEntity)
				{
					m_targetEntity.set(opposingFaction.get().getController(), targetEntity->getID());
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
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID());
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
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID());
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map, m_AABB))
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position, 270.0f);
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.getController(), getID(),
					opposingFaction.getController(), targetEntity->getID(), TURRET_DAMAGE, m_position, targetEntity->getPosition()));
				
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