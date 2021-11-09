#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "PathFinding.h"
#include "ModelManager.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "GameEvents.h"
#include "FactionHandler.h"
#include "PathFinding.h"
#include "glm/gtx/vector_angle.hpp"
#include "GameMessages.h"
#include "GameMessenger.h"
#ifdef RENDER_PATHING
#include "RenderPrimitiveMesh.h"
#endif // RENDER_PATHING

namespace
{
	constexpr float MOVEMENT_SPEED = 10.f;
	constexpr float TIME_BETWEEN_ATTACK = 1.0f;
	constexpr int DAMAGE = 1;
}

Unit::Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map,
	FactionHandler& factionHandler)
	: MovingEntity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit, 
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntity()
{
	broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_position, getID() });
}

Unit::Unit(Faction & owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & startingRotation, 
	const glm::vec3 & destination, FactionHandler& factionHandler, const Map& map)
	: MovingEntity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit,
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntity()
{
	moveTo(destination, map, factionHandler);
}

Unit::~Unit()
{
	if (!m_movementPath.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_movementPath.front(), getID() });
	}
	else
	{
		assert(Globals::isOnMiddlePosition(m_position));
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_position, getID() });
	}
}

TargetEntity Unit::getTargetEntity() const
{
	return m_targetEntity;
}

const Faction& Unit::getOwningFaction() const
{
	return m_owningFaction;
}

const std::vector<glm::vec3>& Unit::getMovementPath() const
{
	return m_movementPath;
}

float Unit::getAttackRange() const
{
	return Globals::UNIT_ATTACK_RANGE;
}

eFactionController Unit::getOwningFactionController() const
{
	return m_owningFaction.get().getController();
}

eUnitState Unit::getCurrentState() const
{
	return m_currentState;
}

void Unit::moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map, FactionHandler& factionHandler)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movementPath, m_position);
	if (!m_movementPath.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_movementPath.front(), getID() });
	}
	bool attackPositionFound = PathFinding::getInstance().setUnitAttackPosition(*this, targetEntity, m_movementPath, map, factionHandler);
	if (attackPositionFound)
	{
		if (!m_movementPath.empty())
		{
			switchToState(eUnitState::Moving, map, factionHandler, &targetEntity, &targetFaction);
		}
		else if(previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movementPath, map, 
				createAdjacentPositions(map, factionHandler, *this));
			switchToState(eUnitState::Moving, map, factionHandler, &targetEntity, &targetFaction);
		}
		else
		{
			switchToState(eUnitState::AttackingTarget, map, factionHandler, &targetEntity, &targetFaction);
		}
	}
	else
	{
		if (m_movementPath.empty())
		{
			if (previousDestination != m_position)
			{
				PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movementPath, map, 
					createAdjacentPositions(map, factionHandler, *this));
				switchToState(eUnitState::Moving, map, factionHandler);
			}
		}
		else
		{
			switchToState(eUnitState::Idle, map, factionHandler);
		}	
	}
}

void Unit::moveTo(const glm::vec3& destination, const Map& map, FactionHandler& factionHandler, eUnitState state)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movementPath, m_position);
	if (!m_movementPath.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_movementPath.front(), getID() });
	}

	PathFinding::getInstance().getPathToPosition(*this, destination, m_movementPath, map, createAdjacentPositions(map, factionHandler, *this));
	if (!m_movementPath.empty())
	{
		switchToState(state, map, factionHandler);
	}
	else
	{
		if (previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movementPath, map,
				createAdjacentPositions(map, factionHandler, *this));

			switchToState(state, map, factionHandler);
		}
		else
		{
			switchToState(eUnitState::Idle, map, factionHandler);
		}
	}
}

void Unit::update(float deltaTime, FactionHandler& factionHandler, const Map& map, const Timer& unitStateHandlerTimer)
{
	Entity::update(deltaTime);

	if (!m_movementPath.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_movementPath.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		setPosition(newPosition);

		if (m_position == m_movementPath.back())
		{
			assert(Globals::isOnMiddlePosition(m_position));
			m_movementPath.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_movementPath.empty() && m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (unitStateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.get().getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, Globals::UNIT_ATTACK_RANGE, true);
				if (targetEntity)
				{
					moveToAttackPosition(*targetEntity, opposingFaction, map, factionHandler);
					break;
				}
			}
		}
		break;
	case eUnitState::Moving:
		if (m_targetEntity.getID() != Globals::INVALID_ENTITY_ID)
		{
			if (unitStateHandlerTimer.isExpired())
			{
				const Faction* targetFaction = nullptr;
				const Entity* targetEntity = nullptr;
				if (targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController()))
				{
					targetEntity = targetFaction->getEntity(m_targetEntity.getID(), m_targetEntity.getType());
				}

				if (targetEntity)
				{
					assert(targetFaction);
					if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
						m_movementPath.empty())
					{
						moveToAttackPosition(*targetEntity, *targetFaction, map, factionHandler);
					}
				}
				else
				{
					if (!m_movementPath.empty())
					{
						moveTo(m_movementPath.front(), map, factionHandler, eUnitState::Moving);
					}
					else
					{
						switchToState(eUnitState::Idle, map, factionHandler);
					}
				}
			}
		}
		else if (m_movementPath.empty())
		{
			if (!m_destinationQueue.empty())
			{
				glm::vec3 destination = m_destinationQueue.front();
				m_destinationQueue.pop_front();
				moveTo(destination, map, factionHandler);
			}
			else
			{
				switchToState(eUnitState::Idle, map, factionHandler);
			}
		}
		break;
	case eUnitState::AttackMoving:
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (unitStateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.get().getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, Globals::UNIT_ATTACK_RANGE);
				if (targetEntity && PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map))
				{
					moveToAttackPosition(*targetEntity, opposingFaction, map, factionHandler);
				}
			}
		}
		if (m_movementPath.empty())
		{
			switchToState(eUnitState::Idle, map, factionHandler);
		}
		break;
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position) && 
			m_movementPath.empty() && 
			m_targetEntity.getID() != Globals::INVALID_ENTITY_ID);
		
		if (unitStateHandlerTimer.isExpired())
		{
			if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
			{
				if (const Faction* targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController()))
				{
					const Entity* targetEntity = targetFaction->getEntity(m_targetEntity.getID(), m_targetEntity.getType());
					if (!targetEntity)
					{
						targetEntity = targetFaction->getEntity(m_position, Globals::UNIT_ATTACK_RANGE);
						if (!targetEntity)
						{
							switchToState(eUnitState::Idle, map, factionHandler);
							break;
						}
						else
						{
							m_targetEntity.set(targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType());
						}
					}
					else
					{
						if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
							!PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map))
						{
							moveToAttackPosition(*targetEntity, *targetFaction, map, factionHandler);
						}
						else if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE)
						{
							m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
						}
					}
				}
			}
			else
			{
				switchToState(eUnitState::Idle, map, factionHandler);
				break;
			}
		}

		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			if (const Faction* targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController()))
			{
				const Entity* targetEntity = targetFaction->getEntity(m_targetEntity.getID(), m_targetEntity.getType());
				if (targetEntity && Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE)
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.get().getController(), getID(),
						getEntityType(), targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType(),
						DAMAGE, m_position, targetEntity->getPosition()));

					m_attackTimer.resetElaspedTime();
				}
			}
		}
		else if (!factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			switchToState(eUnitState::Idle, map, factionHandler);
			break;
		}
	
		break;
	}

	m_attackTimer.update(deltaTime);
}

void Unit::takeDamage(const TakeDamageEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
	Entity::takeDamage(gameEvent, map, factionHandler);	
	if (!isDead())
	{
		m_owningFaction.get().onUnitTakenDamage(gameEvent, *this, map, factionHandler);
	}
}

void Unit::switchToState(eUnitState newState, const Map& map, FactionHandler& factionHandler, 
	const Entity* targetEntity, const Faction* targetFaction)
{
	assert(targetEntity && targetFaction || !targetEntity && !targetFaction);

	//On Exit current state
	switch (m_currentState)
	{
	case eUnitState::Idle:
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position));
		if (newState == eUnitState::Moving || newState == eUnitState::AttackMoving)
		{
			broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_position, getID() });
		}
		break;
	case eUnitState::Moving:
		if (m_currentState != newState)
		{
			clearDestinationQueue();
		}
		break;
	case eUnitState::AttackMoving:
		break;
	default:
		assert(false);
	}

	eUnitState currentState = m_currentState;
	m_currentState = newState;
	//On enter new state
	switch (newState)
	{
	case eUnitState::Idle:
		m_targetEntity.reset();
		m_movementPath.clear();
		m_owningFaction.get().onUnitEnteredIdleState(*this, map, factionHandler);
		break;
	case eUnitState::AttackMoving:
		assert(!m_movementPath.empty());
		broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_movementPath.front(), getID()  });
		m_targetEntity.reset();
		break;
	case eUnitState::AttackingTarget:
		assert(m_movementPath.empty());
		if (currentState != newState)
		{
			m_attackTimer.resetElaspedTime();
		}
		assert(targetEntity && targetFaction);
		m_targetEntity.set(targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType());
		break;
	case eUnitState::Moving:
		assert(!m_movementPath.empty());
		if (targetEntity && targetFaction)
		{
			m_targetEntity.set(targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType());
		}
		else
		{
			m_targetEntity.reset();
		}
		broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_movementPath.front(), getID() });
		break;
	default:
		assert(false);
	}
}