#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "PathFinding.h"
#include "ModelManager.h"
#include "UniqueEntityIDDistributer.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "GameEvent.h"
#include "FactionHandler.h"
#include "PathFindingLocator.h"
#include "PathFinding.h"
#include "glm/gtx/vector_angle.hpp"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr float UNIT_GRID_ATTACK_RANGE = 5.0f;
	constexpr float UNIT_ATTACK_RANGE = UNIT_GRID_ATTACK_RANGE * Globals::NODE_SIZE;

	constexpr float TIME_BETWEEN_ATTACK = 1.0f;
	constexpr float TIME_BETWEEN_STATE = 0.25f;
	constexpr int DAMAGE = 1;

#ifdef RENDER_PATHING
	constexpr glm::vec3 PATH_COLOUR = { 1.0f, 0.27f, 0.0f };
	constexpr float PATH_OPACITY = 0.25f;

	void generateRenderPath(const std::vector<glm::vec3>& path, Mesh& mesh)
	{
		constexpr std::array<glm::vec3, 4> CUBE_FACE_TOP =
		{
			glm::vec3(0.0f, 0.0f, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f)
		};

		mesh.indices.clear();
		mesh.vertices.clear();

		int elementCount = 0;
		for (const auto& pathNode : path)
		{
			for (const auto& i : CUBE_FACE_TOP)
			{
				glm::vec3 position = { pathNode + i };
				position.x -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				mesh.vertices.emplace_back(position);
			}

			for (unsigned int i : Globals::CUBE_FACE_INDICIES)
			{
				mesh.indices.push_back(i + elementCount);
			}

			elementCount += Globals::CUBE_FACE_INDICIE_COUNT;
		}

		mesh.attachToVAO();
	};
#endif // RENDER_PATHING
}

Unit::Unit(const Faction& owningFaction, const glm::vec3& startingPosition, eEntityType entityType, int health, const Model& model)
	: Entity(model, startingPosition, entityType, health),
	m_owningFaction(owningFaction),
	m_pathToPosition(),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_stateHandlerTimer(TIME_BETWEEN_STATE, true),
	m_targetEntity()
{}

Unit::Unit(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, 
	const Map & map, eEntityType entityType, int health, const Model& model)
	: Entity(model, startingPosition, entityType, health),
	m_owningFaction(owningFaction),
	m_pathToPosition(),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_stateHandlerTimer(TIME_BETWEEN_STATE, true),
	m_targetEntity()
{
	moveTo(destinationPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

const std::vector<glm::vec3>& Unit::getPathToPosition() const
{
	return m_pathToPosition;
}

float Unit::getGridAttackRange() const
{
	return UNIT_GRID_ATTACK_RANGE;
}

float Unit::getAttackRange() const
{
	return UNIT_ATTACK_RANGE;
}

bool Unit::isPathEmpty() const
{
	return m_pathToPosition.empty();
}

eFactionController Unit::getOwningFactionController() const
{
	return m_owningFaction.getController();
}

const glm::vec3& Unit::getDestination() const
{
	assert(!isPathEmpty());
	return m_pathToPosition.front();
}

eUnitState Unit::getCurrentState() const
{
	return m_currentState;
}

void Unit::resetTarget()
{
	m_targetEntity.reset();
}

void Unit::moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map, 
	FactionHandler& factionHandler)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	PathFindingLocator::get().setUnitAttackPosition(*this, targetEntity, m_pathToPosition, map, 
		m_owningFaction.getUnits(), factionHandler);
	if (!m_pathToPosition.empty())
	{
		m_targetEntity.set(targetFaction.getController(), targetEntity.getID());
		switchToState(eUnitState::Moving, map);
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(eUnitState::Moving, map);
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}
	}
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, eUnitState state)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	PathFindingLocator::get().getPathToPosition(*this, destinationPosition, m_pathToPosition, adjacentPositions,
		m_owningFaction.getUnits(), map);
	if (!m_pathToPosition.empty())
	{
		switchToState(state, map);
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(state, map);
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}
	}
}

void Unit::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	m_attackTimer.update(deltaTime);
	m_stateHandlerTimer.update(deltaTime);

	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		m_position = newPosition;
		m_AABB.update(m_position);

		if (m_position == m_pathToPosition.back())
		{
			m_pathToPosition.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired() && getEntityType() == eEntityType::Unit)
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, UNIT_ATTACK_RANGE);
				if (targetEntity)
				{
					if (PathFindingLocator::get().isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						switchToState(eUnitState::AttackingTarget, map);
					}
					else if(!Globals::UNIT_TYPES.isMatch(targetEntity->getEntityType()))
					{
						moveToAttackPosition(*targetEntity, opposingFaction.get(), map, factionHandler);
					}

					m_targetEntity.set(opposingFaction.get().getController(), targetEntity->getID());
					break;
				}
				else
				{
					m_targetEntity.reset();
				}
			}
		}
		break;
	case eUnitState::Moving:
		if (m_targetEntity.getID() != Globals::INVALID_ENTITY_ID)
		{
			if (m_stateHandlerTimer.isExpired() &&
				(!factionHandler.isFactionActive(m_targetEntity.getFactionController()) ||
				!factionHandler.getFaction(m_targetEntity.getFactionController()).getEntity(m_targetEntity.getID())))
			{
				m_targetEntity.reset();
			}

			if (m_pathToPosition.empty())
			{
				switchToState(eUnitState::AttackingTarget, map);
			}
		}
		else if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Idle, map);
		}
		break;
	case eUnitState::AttackMoving:
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, UNIT_ATTACK_RANGE);
				if (targetEntity && PathFindingLocator::get().isTargetInLineOfSight(m_position, *targetEntity, map))
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
	case eUnitState::SetAttackPosition:
		assert(m_targetEntity.getID() != Globals::INVALID_ENTITY_ID);
		if (m_stateHandlerTimer.isExpired())
		{
			const Entity* targetEntity = nullptr;
			if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
			{
				targetEntity = factionHandler.getFaction(m_targetEntity.getFactionController()).getEntity(m_targetEntity.getID());
				if (targetEntity)
				{
					const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
					moveToAttackPosition(*targetEntity, targetFaction, map, factionHandler);
				}
			}

			if (!targetEntity)
			{
				m_targetEntity.reset();
				switchToState(eUnitState::Moving, map);
			}
		}
		break;
	case eUnitState::AttackingTarget:
		assert(m_targetEntity.getID() != Globals::INVALID_ENTITY_ID && m_pathToPosition.empty());
		if (m_attackTimer.isExpired())
		{
			if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
			{
				const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
				const Entity* targetEntity = targetFaction.getEntity(m_targetEntity.getID());
				if (!targetEntity)
				{
					targetEntity = targetFaction.getEntity(m_position, UNIT_ATTACK_RANGE);
					if (!targetEntity)
					{
						switchToState(eUnitState::Idle, map);
					}
					else
					{
						m_targetEntity.set(targetFaction.getController(), targetEntity->getID());
					}
				}
				else
				{
					if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE ||
						!PathFindingLocator::get().isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						moveToAttackPosition(*targetEntity, targetFaction, map, factionHandler);
					}
					else if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
					{
						m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
						GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.getController(), getID(),
							targetFaction.getController(), targetEntity->getID(), DAMAGE, m_position, targetEntity->getPosition()));
					}
				}
			}
			else
			{
				switchToState(eUnitState::Idle, map);
			}
		}
	
		break;
	}

	if (m_attackTimer.isExpired())
	{
		m_attackTimer.resetElaspedTime();
	}

	if (m_stateHandlerTimer.isExpired())
	{
		m_stateHandlerTimer.resetElaspedTime();
	}
}

void Unit::reduceHealth(const GameEvent_4& gameEvent, FactionHandler& factionHandler, const Map& map)
{
	Entity::reduceHealth(gameEvent);
	
	if (!isDead() && m_owningFaction.getController() != eFactionController::Player)
	{
		bool changeTargetEntity = false;
		if(m_targetEntity.getID() != Globals::INVALID_ENTITY_ID &&
			factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& opposingFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntity.getID());
			if (!targetEntity)
			{
				if (gameEvent.senderID != m_targetEntity.getID())
				{
					changeTargetEntity = true;
				}
				else
				{
					m_targetEntity.reset();
				}
			}
			else if(Globals::getSqrDistance(targetEntity->getPosition(), m_position) > glm::pow(UNIT_ATTACK_RANGE, 2))
			{
				changeTargetEntity = true;
			}
			else if(!Globals::ATTACKING_ENTITY_TYPES.isMatch(targetEntity->getEntityType()))
			{
				changeTargetEntity = true;
			}
		}
		else
		{
			changeTargetEntity = true;
		}

		if (changeTargetEntity && factionHandler.isFactionActive(gameEvent.senderFaction))
		{
			const Faction& opposingFaction = factionHandler.getFaction(gameEvent.senderFaction);
			const Entity* targetEntity = opposingFaction.getEntity(gameEvent.senderID);
			if (targetEntity)
			{
				moveToAttackPosition(*targetEntity, opposingFaction, map, factionHandler);
			}	
		}
	}
}

void Unit::switchToState(eUnitState newState, const Map& map, const Entity* targetEntity)
{
	switch (newState)
	{
	case eUnitState::Idle:
	case eUnitState::MovingToMinerals:
	case eUnitState::ReturningMineralsToHQ:
	case eUnitState::MovingToBuildingPosition:
	case eUnitState::MovingToRepairPosition:
	case eUnitState::Building:
	case eUnitState::Harvesting:
	case eUnitState::Repairing:
	case eUnitState::AttackMoving:
		m_targetEntity.reset();
		break;
	case eUnitState::AttackingTarget:
		if (!Globals::isOnMiddlePosition(m_position) && targetEntity)
		{
			m_pathToPosition.emplace_back(PathFindingLocator::get().getClosestPositionFromUnitToTarget(*this, *targetEntity, m_pathToPosition,
				map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }));
		}
		break;

	case eUnitState::SetAttackPosition:
	case eUnitState::Moving:
		break;
	default:
		assert(false);
	}

	m_currentState = newState;
}

#ifdef RENDER_PATHING
void Unit::renderPathMesh(ShaderHandler& shaderHandler)
{
	if (!m_pathToPosition.empty())
	{	
		shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
		shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", PATH_OPACITY);
		generateRenderPath(m_pathToPosition, m_renderPathMesh);
		m_renderPathMesh.renderDebugMesh(shaderHandler);
	}
}
#endif // RENDER_PATHING