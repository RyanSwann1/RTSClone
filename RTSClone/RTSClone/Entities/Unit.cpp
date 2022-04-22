#include "Unit.h"
#include "Graphics/ShaderHandler.h"
#include "Graphics/Model.h"
#include "Core/PathFinding.h"
#include "Graphics/ModelManager.h"
#include "Factions/Faction.h"
#include "Events/GameEvents.h"
#include "Factions/FactionHandler.h"
#include "Core/PathFinding.h"
#include "glm/gtx/vector_angle.hpp"
#include "Events/GameMessages.h"
#include "Events/GameMessenger.h"
#include "Core/Level.h"
#ifdef RENDER_PATHING
#include "Graphics/RenderPrimitiveMesh.h"
#endif // RENDER_PATHING

namespace
{
	constexpr float MOVEMENT_SPEED = 10.f;
	constexpr float TIME_BETWEEN_ATTACK = 1.0f;
	constexpr int DAMAGE = 1;
}

Unit::Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map)
	: Entity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit, 
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction.getController()),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{
	broadcast<GameMessages::AddUnitPositionToMap>({ m_position, getID() });
}

Unit::Unit(Faction & owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & startingRotation, 
	const glm::vec3 & destination, const Map& map)
	: Entity(ModelManager::getInstance().getModel(UNIT_MODEL_NAME), startingPosition, eEntityType::Unit,
		Globals::UNIT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction.getController()),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{
	move_to(destination, map);
}

Unit::~Unit()
{
	if (getID() != INVALID_ENTITY_ID)
	{
		if (!m_movement.path.empty())
		{
			broadcast<GameMessages::RemoveUnitPositionFromMap>({ m_movement.path.front(), getID() });
		}
		else
		{
			assert(Globals::isOnMiddlePosition(m_position));
			broadcast<GameMessages::RemoveUnitPositionFromMap>({ m_position, getID() });
		}
	}
}

std::optional<TargetEntity> Unit::getTargetEntity() const
{
	return m_target;
}

const std::vector<glm::vec3>& Unit::getMovementPath() const
{
	return m_movement.path;
}

float Unit::getAttackRange() const
{
	return Globals::UNIT_ATTACK_RANGE;
}

eUnitState Unit::getCurrentState() const
{
	return m_currentState;
}

bool Unit::is_singular_selectable_only() const
{
	return false;
}

void Unit::add_destination(const glm::vec3& position, const Map& map)
{
	if (m_movement.path.empty())
	{
		move_to(position, map);
	}
	else
	{
		m_movement.destinations.push(position);
	}
}

void Unit::clear_destinations()
{
	m_movement.destinations = {};
}

void Unit::attack_target(const Entity& targetEntity, const eFactionController targetController, const Map& map)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);
	if (!m_movement.path.empty())
	{
		broadcast<GameMessages::RemoveUnitPositionFromMap>({ m_movement.path.front(), getID() });
	}
	bool attackPositionFound = PathFinding::getInstance().setUnitAttackPosition(*this, targetEntity, m_movement.path, map);
	if (attackPositionFound)
	{
		m_target = { targetController, targetEntity.getID()};
		if (!m_movement.path.empty())
		{
			switchToState(eUnitState::Moving);
		}
		else if(previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movement.path, map, 
				createAdjacentPositions(map, *this));
			switchToState(eUnitState::Moving);
		}
		else
		{
			switchToState(eUnitState::AttackingTarget);
		}
	}
	else
	{
		if (m_movement.path.empty())
		{
			if (previousDestination != m_position)
			{
				PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movement.path, map, 
					createAdjacentPositions(map, *this));
				switchToState(eUnitState::Moving);
			}
		}
		else
		{
			switchToState(eUnitState::Idle);
		}	
	}
}

void Unit::move_to(const glm::vec3& destination, const Map& map)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);
	if (!m_movement.path.empty())
	{
		broadcast<GameMessages::RemoveUnitPositionFromMap>({ m_movement.path.front(), getID() });
	}

	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map, *this));
	if (!m_movement.path.empty())
	{
		switchToState(eUnitState::Moving);
	}
	else
	{
		if (previousDestination != m_position)
		{
			PathFinding::getInstance().getPathToPosition(*this, previousDestination, m_movement.path, map,
				createAdjacentPositions(map, *this));

			switchToState(eUnitState::Moving);
		}
		else
		{
			switchToState(eUnitState::Idle);
		}
	}
}

void Unit::update(float deltaTime, const FactionHandler& factionHandler, const Map& map)
{
	Entity::update(deltaTime);

	if (!m_movement.path.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_movement.path.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		setPosition(newPosition);

		if (m_position == m_movement.path.back())
		{
			assert(Globals::isOnMiddlePosition(m_position));
			m_movement.path.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_movement.path.empty() && !m_target);
		break;
	case eUnitState::Moving:
		if (m_movement.path.empty())
		{
			if (!m_movement.destinations.empty())
			{
				glm::vec3 destination = m_movement.destinations.front();
				m_movement.destinations.pop();
				move_to(destination, map);
			}
			else
			{
				switchToState(eUnitState::Idle);
			}
		}
		break;
	case eUnitState::AttackMoving:
		assert(!m_target);
		if (m_movement.path.empty())
		{
			switchToState(eUnitState::Idle);
		}
		break;
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position) && 
			m_movement.path.empty() && 
			m_target);

		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_target->controller))
		{
			if (const Faction* targetFaction = factionHandler.getFaction(m_target->controller))
			{
				const Entity* targetEntity = targetFaction->get_entity(m_target->ID);
				if (targetEntity && Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE)
				{
					Level::add_event(GameEvent::create<SpawnProjectileEvent>({ m_owningFaction, getID(),
						getEntityType(), targetFaction->getController(), targetEntity->getID(), targetEntity->getEntityType(),
						DAMAGE, m_position, targetEntity->getPosition() }));

					m_attackTimer.resetElaspedTime();
				}
			}
		}
		else if (!factionHandler.isFactionActive(m_target->controller))
		{
			switchToState(eUnitState::Idle);
			break;
		}
	
		break;
	}

	m_attackTimer.update(deltaTime);
}

void Unit::delayed_update(const FactionHandler& factionHandler, const Map& map)
{
	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_movement.path.empty() && !m_target);
		for (const Faction* opposingFaction : factionHandler.getOpposingFactions(m_owningFaction))
		{
			if (!opposingFaction)
			{
				continue;
			}

			const Entity* targetEntity = opposingFaction->getEntity(m_position, Globals::UNIT_ATTACK_RANGE, true);
			if (targetEntity)
			{
				attack_target(*targetEntity, opposingFaction->getController(), map);
				break;
			}
		}
		break;
	case eUnitState::Moving:
		if (m_target)
		{
			const Faction* targetFaction = nullptr;
			const Entity* targetEntity = nullptr;
			if (targetFaction = factionHandler.getFaction(m_target->controller))
			{
				targetEntity = targetFaction->get_entity(m_target->ID);
			}

			if (targetEntity)
			{
				assert(targetFaction);
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
					m_movement.path.empty())
				{
					attack_target(*targetEntity, targetFaction->getController(), map);
				}
			}
			else
			{
				if (!m_movement.path.empty())
				{
					move_to(m_movement.path.front(), map);
				}
				else
				{
					switchToState(eUnitState::Idle);
				}
			}
		}
		break;
	case eUnitState::AttackMoving:
		assert(!m_target);
		for (const Faction* opposingFaction : factionHandler.getOpposingFactions(m_owningFaction))
		{
			if (!opposingFaction)
			{
				continue;
			}

			const Entity* targetEntity = opposingFaction->getEntity(m_position, Globals::UNIT_ATTACK_RANGE);
			if (targetEntity && PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map))
			{
				attack_target(*targetEntity, opposingFaction->getController(), map);
			}
		}
		break;
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position) &&
			m_movement.path.empty() &&
			m_target);

		if (factionHandler.isFactionActive(m_target->controller))
		{
			if (const Faction* targetFaction = factionHandler.getFaction(m_target->controller))
			{
				const Entity* targetEntity = targetFaction->get_entity(m_target->ID);
				if (!targetEntity)
				{
					targetEntity = targetFaction->getEntity(m_position, Globals::UNIT_ATTACK_RANGE);
					if (!targetEntity)
					{
						switchToState(eUnitState::Idle);
						break;
					}
					else
					{
						m_target = { targetFaction->getController(), targetEntity->getID() };
					}
				}
				else
				{
					if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > Globals::UNIT_ATTACK_RANGE * Globals::UNIT_ATTACK_RANGE ||
						!PathFinding::getInstance().isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						attack_target(*targetEntity, targetFaction->getController(), map);
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
			switchToState(eUnitState::Idle);
			break;
		}

		break;
	}
}

void Unit::revalidate_movement_path(const Map& map)
{
	if (!m_movement.path.empty())
	{
		move_to(m_movement.path.front(), map);
	}
}

#ifdef RENDER_PATHING
void Unit::render_path(ShaderHandler& shaderHandler) 
{
	RenderPrimitiveMesh::render(shaderHandler, m_movement.path, m_movement.pathMesh);
}
#endif // RENDER_PATHING

void Unit::switchToState(eUnitState newState)
{
	//On Exit current state
	switch (m_currentState)
	{
	case eUnitState::Idle:
	case eUnitState::AttackingTarget:
		assert(Globals::isOnMiddlePosition(m_position));
		if (newState == eUnitState::Moving || newState == eUnitState::AttackMoving)
		{
			broadcast<GameMessages::RemoveUnitPositionFromMap>({ m_position, getID() });
		}
		else if (newState != eUnitState::AttackMoving)
		{
			m_target = std::nullopt;
		}
		break;
	case eUnitState::Moving:
		if (m_currentState != newState)
		{
			m_movement.path.clear();
		}
		break;
	case eUnitState::AttackMoving:
		if (newState != eUnitState::AttackingTarget)
		{
			m_target = std::nullopt;
		}
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
		m_target.reset();
		m_movement.path.clear();
		Level::add_event(GameEvent::create<EntityIdleEvent>({ getID(), m_owningFaction }));
		break;
	case eUnitState::AttackMoving:
		assert(!m_movement.path.empty());
		broadcast<GameMessages::AddUnitPositionToMap>({ m_movement.path.front(), getID()  });
		m_target.reset();
		break;
	case eUnitState::AttackingTarget:
		assert(m_movement.path.empty());
		if (currentState != newState)
		{
			m_attackTimer.resetElaspedTime();
		}
		break;
	case eUnitState::Moving:
		assert(!m_movement.path.empty());
		broadcast<GameMessages::AddUnitPositionToMap>({ m_movement.path.front(), getID() });
		break;
	default:
		assert(false);
	}
}