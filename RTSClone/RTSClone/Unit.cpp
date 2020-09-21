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

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr float UNIT_ATTACK_RANGE = 5.0f * Globals::NODE_SIZE;
	constexpr float TIME_BETWEEN_ATTACK = 1.0f;
	constexpr float TIME_BETWEEN_MOVEMENT_STATE = 0.5f;
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

		mesh.m_indices.clear();
		mesh.m_vertices.clear();

		int elementCount = 0;
		for (const auto& pathNode : path)
		{
			for (const auto& i : CUBE_FACE_TOP)
			{
				glm::vec3 position = { pathNode + i };
				position.x -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				mesh.m_vertices.emplace_back(position);
			}

			for (unsigned int i : Globals::CUBE_FACE_INDICIES)
			{
				mesh.m_indices.push_back(i + elementCount);
			}

			elementCount += Globals::CUBE_FACE_INDICIE_COUNT;
		}

		mesh.attachToVAO();
	};
#endif // RENDER_PATHING

	bool isTargetInLineOfSight(const glm::vec3& unitPosition, const Entity& targetEntity, const Map& map)
	{
		glm::vec3 direction = glm::normalize(targetEntity.getPosition() - unitPosition);
		constexpr float step = 0.5f;
		float distance = glm::distance(targetEntity.getPosition(), unitPosition);
		bool targetEntityVisible = true;

		for (int i = 0; i < std::ceil(distance / step); ++i)
		{
			glm::vec3 position = unitPosition + direction * static_cast<float>(i);
			if (targetEntity.getAABB().contains(position))
			{
				break;
			}
			else if (map.isPositionOccupied(position))
			{
				targetEntityVisible = false;
				break;
			}
		}

		return targetEntityVisible;
	}
}

Unit::Unit(const Faction& owningFaction, const glm::vec3& startingPosition, eEntityType entityType, int health)
	: Entity(startingPosition, entityType, health),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition(),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_movementStateTimer(TIME_BETWEEN_MOVEMENT_STATE, true),
	m_target()
{}

Unit::Unit(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, 
	const Map & map, eEntityType entityType, int health)
	: Entity(startingPosition, entityType, health),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition(),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_movementStateTimer(TIME_BETWEEN_MOVEMENT_STATE, true),
	m_target()
{
	moveTo(destinationPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

bool Unit::isPathEmpty() const
{
	return m_pathToPosition.empty();
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
	m_target.reset();
}

void Unit::setTarget(eFactionController targetFaction, int targetID)
{
	m_target.set(targetFaction, targetID);
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
	eUnitState state)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, adjacentPositions, 
		m_owningFaction.getUnits(), map);
	if (!m_pathToPosition.empty())
	{
		switchToState(state);
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(state);
		}
		else
		{
			switchToState(eUnitState::Idle);
		}
	}
}

void Unit::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	m_attackTimer.update(deltaTime);
	m_movementStateTimer.update(deltaTime);

	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_front = glm::normalize(glm::vec3(newPosition - m_position));
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
		assert(m_target.getID() == Globals::INVALID_ENTITY_ID);
		if (m_attackTimer.isExpired() && getEntityType() == eEntityType::Unit)
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction->getEntity(m_position, UNIT_ATTACK_RANGE);
				if (targetEntity)
				{
					if (isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						switchToState(eUnitState::AttackingTarget);
					}
					else
					{
						moveTo(targetEntity->getPosition(), map, [&](const glm::ivec2& position)
							{ return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); });
					}

					m_target.set(opposingFaction->getController(), targetEntity->getID());
					break;
				}
			}
		}
		break;
	case eUnitState::Moving:
		if (m_movementStateTimer.isExpired() && Globals::isEntityIDValid(m_target.getID()))
		{
			const Entity* targetEntity = nullptr;
			if (factionHandler.isFactionActive(m_target.getFactionController()))
			{
				targetEntity = factionHandler.getFaction(m_target.getFactionController()).getEntity(m_target.getID());
			}
			if (targetEntity)
			{
				if(Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
				{
					if (!isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						moveTo(targetEntity->getPosition(), map, [&](const glm::ivec2& position)
							{ return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }, eUnitState::Moving);
					}
					else
					{
						switchToState(eUnitState::AttackingTarget);
						if (!Globals::isOnMiddlePosition(m_position))
						{
							m_pathToPosition.clear();
							m_pathToPosition.emplace_back(PathFinding::getInstance().getClosestPositionFromUnitToTarget(*this, *targetEntity, m_pathToPosition,
								[&](const glm::ivec2& position) { return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }));
						}
					}	
				}
			}
			else
			{
				switchToState(eUnitState::Idle);
			}
		}
		else if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Idle);
		}
		break;
	case eUnitState::AttackMoving:
		if (m_movementStateTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction->getEntity(m_position, UNIT_ATTACK_RANGE);
				if (targetEntity && isTargetInLineOfSight(m_position, *targetEntity, map))
				{
					m_target.set(opposingFaction->getController(), targetEntity->getID());
					switchToState(eUnitState::AttackingTarget);
					if (!Globals::isOnMiddlePosition(m_position))
					{
						m_pathToPosition.clear();
						m_pathToPosition.emplace_back(PathFinding::getInstance().getClosestPositionFromUnitToTarget(*this, *targetEntity, m_pathToPosition,
							[&](const glm::ivec2& position) { return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }));
					}
				}
			}
		}
		else if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Idle);
		}
		break;
	case eUnitState::AttackingTarget:
		assert(m_target.getID() != Globals::INVALID_ENTITY_ID);
		if (m_attackTimer.isExpired())
		{
			if (factionHandler.isFactionActive(m_target.getFactionController()))
			{
				const Faction& opposingFaction = factionHandler.getFaction(m_target.getFactionController());
				const Entity* targetEntity = opposingFaction.getEntity(m_target.getID());
				if (!targetEntity)
				{
					targetEntity = opposingFaction.getEntity(m_position, UNIT_ATTACK_RANGE);
					if (!targetEntity)
					{
						switchToState(eUnitState::Idle);
					}
					else
					{
						m_target.set(opposingFaction.getController(), targetEntity->getID());
					}
				}

				if (targetEntity)
				{
					if ((Globals::getSqrDistance(targetEntity->getPosition(), m_position) > UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE) ||
						!isTargetInLineOfSight(m_position, *targetEntity, map))
					{
						moveTo(targetEntity->getPosition(), map, [&](const glm::ivec2& position)
							{ return getAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); });
					}
					else if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
					{
						GameEventHandler::getInstance().addEvent({ eGameEventType::SpawnProjectile, m_owningFaction.getController(), getID(),
							opposingFaction.getController(), targetEntity->getID(), DAMAGE, m_position, targetEntity->getPosition() });
					}
				}
			}
			else
			{
				switchToState(eUnitState::Idle);
			}
		}
	
		break;
	}

	if (m_attackTimer.isExpired())
	{
		m_attackTimer.resetElaspedTime();
	}

	if (m_movementStateTimer.isExpired())
	{
		m_movementStateTimer.resetElaspedTime();
	}
}

void Unit::switchToState(eUnitState newState)
{
	switch (newState)
	{
	case eUnitState::Idle:
	case eUnitState::MovingToMinerals:
	case eUnitState::ReturningMineralsToHQ:
	case eUnitState::MovingToBuildingPosition:
	case eUnitState::Building:
		m_target.reset();
		break;
	case eUnitState::Moving:
	case eUnitState::AttackMoving:
	case eUnitState::Harvesting:
	case eUnitState::AttackingTarget:
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
		m_renderPathMesh.render(shaderHandler, isSelected());
	}
}
#endif // RENDER_PATHING