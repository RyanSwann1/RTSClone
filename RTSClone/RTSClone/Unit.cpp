#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "PathFinding.h"
#include "ModelManager.h"
#include "UniqueEntityIDDistributer.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "GameEvent.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr float UNIT_ATTACK_RANGE = 5.0f * Globals::NODE_SIZE;
	constexpr float TIME_BETWEEN_ATTACK = 1.0f;

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
}

Unit::Unit(const Faction& owningFaction, const glm::vec3& startingPosition, eEntityType entityType)
	: Entity(startingPosition, entityType),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition(),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntityID(Globals::INVALID_ENTITY_ID)
{}

Unit::Unit(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Map & map, eEntityType entityType)
	: Entity(startingPosition, entityType),
	m_owningFaction(owningFaction),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition(),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntityID(Globals::INVALID_ENTITY_ID)
{
	moveTo(destinationPosition, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
}

int Unit::getTargetID() const
{
	return m_targetEntityID;
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

void Unit::resetTargetID()
{
	m_targetEntityID = Globals::INVALID_ENTITY_ID;
}

void Unit::setTargetID(int entityTargetID)
{
	assert(entityTargetID != Globals::INVALID_ENTITY_ID);
	m_targetEntityID = entityTargetID;
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map, const GetAllAdjacentPositions& getAdjacentPositions, 
	eUnitState state)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, getAdjacentPositions);
	PathFinding::getInstance().convertPathToWaypoints(m_pathToPosition, *this, m_owningFaction.getUnits(), map);
	if (!m_pathToPosition.empty())
	{
		m_currentState = state;
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			m_currentState = state;
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
	}
}

void Unit::update(float deltaTime, const Faction& opposingFaction, const Map& map)
{
	assert(opposingFaction.getName() != m_owningFaction.getName());
	m_attackTimer.update(deltaTime);

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
		assert(m_targetEntityID == Globals::INVALID_ENTITY_ID && m_pathToPosition.empty());
		if (m_attackTimer.isExpired() && getEntityType() == eEntityType::Unit)
		{
			const Entity* targetEntity = opposingFaction.getEntity(m_position, UNIT_ATTACK_RANGE);
			if (targetEntity)
			{
				m_targetEntityID = targetEntity->getID();
				m_currentState = eUnitState::AttackingTarget;
			}
		}
		break;
	case eUnitState::Moving:
		if (Globals::isEntityIDValid(m_targetEntityID))
		{
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntityID);
			if (targetEntity)
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
				{
					m_currentState = eUnitState::AttackingTarget;

					if (!m_pathToPosition.empty())
					{
						glm::vec3 destination = m_pathToPosition.front();
						m_pathToPosition.clear();

						if (!Globals::isOnMiddlePosition(m_position))
						{
							m_pathToPosition.push_back(PathFinding::getInstance().getClosestPositionToDestination(m_position, destination,
								[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }));
						}
					}
				}
			}
			else
			{
				m_targetEntityID = Globals::INVALID_ENTITY_ID;
				m_currentState = eUnitState::Idle;
				m_pathToPosition.clear();
			}
		}
		else if (m_pathToPosition.empty())
		{
			m_currentState = eUnitState::Idle;
			m_targetEntityID = Globals::INVALID_ENTITY_ID;
		}
		break;
	case eUnitState::AttackMoving:
		if (m_attackTimer.isExpired())
		{
			const Entity* targetEntity = opposingFaction.getEntity(m_position, UNIT_ATTACK_RANGE);
			if (targetEntity)
			{
				m_targetEntityID = targetEntity->getID();
				m_currentState = eUnitState::AttackingTarget;
			}
		}
		break;
	case eUnitState::AttackingTarget:
		assert(m_targetEntityID != Globals::INVALID_ENTITY_ID);

		if (m_attackTimer.isExpired())
		{
			const Entity* targetEntity = opposingFaction.getEntity(m_targetEntityID);
			if (!targetEntity)
			{
				targetEntity = opposingFaction.getEntity(m_position, UNIT_ATTACK_RANGE);
				if (!targetEntity)
				{
					m_targetEntityID = Globals::INVALID_ENTITY_ID;
					m_currentState = eUnitState::Idle;
				}
			}

			if (targetEntity)
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
				{
					GameEventHandler::getInstance().addEvent({ eGameEventType::SpawnProjectile, m_owningFaction.getName(), getID(),
						targetEntity->getID(), m_position, targetEntity->getPosition() });
				}
				else
				{
					//moveTo(targetEntity->getPosition(), map, [&](const glm::ivec2& position) 
					//	{ return getAllAdjacentPositions(position, map, m_owningFaction.getUnits(), *this); }, eUnitState::AttackingTarget);
				}
			}
		}
	
		break;
	}

	if (m_attackTimer.isExpired())
	{
		m_attackTimer.resetElaspedTime();
	}
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