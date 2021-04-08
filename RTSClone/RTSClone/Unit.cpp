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
#include "RenderPathMesh.h"
#endif // RENDER_PATHING

namespace
{
	const float MOVEMENT_SPEED = 7.5f;
	const float TIME_BETWEEN_ATTACK = 1.0f;
	const int DAMAGE = 1;
}

Unit::Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map)
	: Entity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit, 
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_pathToPosition(),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntity()
{
	switchToState(m_currentState, map);
	broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_position, getID() });
}

Unit::Unit(const Faction & owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & startingRotation, 
	const glm::vec3 & destination, FactionHandler& factionHandler, const Map& map)
	: Entity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit,
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_pathToPosition(),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntity()
{
	moveTo(destination, map, factionHandler);
}

Unit::~Unit()
{
	if (!m_pathToPosition.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_pathToPosition.front(), getID() });
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

const std::vector<glm::vec3>& Unit::getPathToPosition() const
{
	return m_pathToPosition;
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
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_pathToPosition, m_position);
	if (!m_pathToPosition.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_pathToPosition.front(), getID() });
	}
	bool attackPositionFound = PathFinding::getInstance().setUnitAttackPosition(*this, targetEntity, m_pathToPosition, map, factionHandler);
	if (attackPositionFound)
	{
		if (!m_pathToPosition.empty())
		{
			switchToState(eUnitState::Moving, map, &targetEntity, &targetFaction);
		}
		else if(previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_pathToPosition, map, factionHandler, m_owningFaction);
			switchToState(eUnitState::Moving, map, &targetEntity, &targetFaction);
		}
		else
		{
			switchToState(eUnitState::AttackingTarget, map, &targetEntity, &targetFaction);
		}
	}
	else
	{
		if (m_pathToPosition.empty())
		{
			if (previousDestination != m_position)
			{
				PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_pathToPosition, map, factionHandler, m_owningFaction);
				switchToState(eUnitState::Moving, map);
			}
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}	
	}
}

void Unit::moveTo(const glm::vec3& destination, const Map& map, FactionHandler& factionHandler, eUnitState state)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_pathToPosition, m_position);
	if (!m_pathToPosition.empty())
	{
		broadcastToMessenger<GameMessages::RemoveUnitPositionFromMap>({ m_pathToPosition.front(), getID() });
	}

	PathFinding::getInstance().getPathToPosition(*this, destination, m_pathToPosition, map, factionHandler, m_owningFaction);
	if (!m_pathToPosition.empty())
	{
		switchToState(state, map);
	}
	else
	{
		if (previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_pathToPosition, map, factionHandler, m_owningFaction);

			switchToState(state, map);
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}
	}
}

void Unit::update(float deltaTime, FactionHandler& factionHandler, const Map& map, const Timer& unitStateHandlerTimer)
{
	Entity::update(deltaTime);

	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		setPosition(newPosition);

		if (m_position == m_pathToPosition.back())
		{
			assert(Globals::isOnMiddlePosition(m_position));
			m_pathToPosition.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_pathToPosition.empty() && m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
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
				const Entity* targetEntity = nullptr;
				if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
				{
					targetEntity = factionHandler.getFaction(m_targetEntity.getFactionController()).getEntity(m_targetEntity.getID(), m_targetEntity.getType());
				}

				if (targetEntity)
				{
					if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
						m_pathToPosition.empty())
					{
						moveToAttackPosition(*targetEntity, factionHandler.getFaction(m_targetEntity.getFactionController()), map, factionHandler);
					}
				}
				else
				{
					if (!m_pathToPosition.empty())
					{
						moveTo(m_pathToPosition.front(), map, factionHandler, eUnitState::Moving);
					}
					else
					{
						switchToState(eUnitState::Idle, map);
					}
				}
			}
		}
		else if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Idle, map);
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
		if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Idle, map);
		}
		break;
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position) && 
			m_pathToPosition.empty() && 
			m_targetEntity.getID() != Globals::INVALID_ENTITY_ID);
		
		if (unitStateHandlerTimer.isExpired())
		{
			if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
			{
				const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
				const Entity* targetEntity = targetFaction.getEntity(m_targetEntity.getID(), m_targetEntity.getType());
				if (!targetEntity)
				{
					targetEntity = targetFaction.getEntity(m_position, Globals::UNIT_ATTACK_RANGE);
					if (!targetEntity)
					{
						switchToState(eUnitState::Idle, map);
						break;
					}
					else
					{
						m_targetEntity.set(targetFaction.getController(), targetEntity->getID(), targetEntity->getEntityType());
					}
				}
				else
				{
					if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
						!PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						moveToAttackPosition(*targetEntity, targetFaction, map, factionHandler);
					}
					else if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE)
					{
						m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
					}
				}
			}
			else
			{
				switchToState(eUnitState::Idle, map);
				break;
			}
		}

		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = targetFaction.getEntity(m_targetEntity.getID(), m_targetEntity.getType());
			if (targetEntity && Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE)
			{
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.get().getController(), getID(),
					getEntityType(), targetFaction.getController(), targetEntity->getID(), DAMAGE, m_position, targetEntity->getPosition()));

				m_attackTimer.resetElaspedTime();
			}
		}
		else if (!factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			switchToState(eUnitState::Idle, map);
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

void Unit::switchToState(eUnitState newState, const Map& map, const Entity* targetEntity, const Faction* targetFaction)
{
	assert(targetEntity && targetFaction || !targetEntity && !targetFaction);

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
	case eUnitState::AttackMoving:
	case eUnitState::Moving:
		break;
	default:
		assert(false);
	}

	//On enter new state
	switch (newState)
	{
	case eUnitState::Idle:
		GameEventHandler::getInstance().gameEvents.push(
			GameEvent::createOnEnteredIdleState(m_owningFaction.get().getController(), getEntityType(), getID()));
		broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_position, getID() });
		m_targetEntity.reset();
		m_pathToPosition.clear();
		break;
	case eUnitState::AttackMoving:
		assert(!m_pathToPosition.empty());
		broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_pathToPosition.front(), getID()  });
		m_targetEntity.reset();
		break;
	case eUnitState::AttackingTarget:
		assert(m_pathToPosition.empty());
		if (m_currentState != newState)
		{
			m_attackTimer.resetElaspedTime();
		}
		assert(targetEntity && targetFaction);
		m_targetEntity.set(targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType());
		break;
	case eUnitState::Moving:
		assert(!m_pathToPosition.empty());
		if (targetEntity && targetFaction)
		{
			m_targetEntity.set(targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType());
		}
		else
		{
			m_targetEntity.reset();
		}
		broadcastToMessenger<GameMessages::AddUnitPositionToMap>({ m_pathToPosition.front(), getID() });
		break;
	default:
		assert(false);
	}

	m_currentState = newState;
}

#ifdef RENDER_PATHING
void Unit::renderPathMesh(ShaderHandler& shaderHandler)
{
	RenderPathMesh::render(shaderHandler, m_pathToPosition, m_renderPathMesh);
}
#endif // RENDER_PATHING