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
#include "PathFinding.h"
#include "glm/gtx/vector_angle.hpp"
#include "GameMessages.h"
#include "GameMessenger.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr float UNIT_GRID_ATTACK_RANGE = 5.0f;
	constexpr float UNIT_ATTACK_RANGE = UNIT_GRID_ATTACK_RANGE * Globals::NODE_SIZE;
	constexpr float TIME_BETWEEN_ATTACK = 1.0f;
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
	m_targetEntity()
{
	if (getEntityType() == eEntityType::Unit)
	{
		GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
	}
}

Unit::Unit(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, 
	const Map & map, eEntityType entityType, int health, const Model& model)
	: Entity(model, startingPosition, entityType, health),
	m_owningFaction(owningFaction),
	m_pathToPosition(),
	m_currentState(eUnitState::Idle),
	m_attackTimer(TIME_BETWEEN_ATTACK, true),
	m_targetEntity()
{
	moveTo(destinationPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

Unit::~Unit()
{
	if (getEntityType() == eEntityType::Unit)
	{
		GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB, getID() });
	}
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
	const Faction& faction)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	if (PathFinding::getInstance().setUnitAttackPosition(*this, targetEntity, m_pathToPosition, map, faction))
	{
		m_targetEntity.set(targetFaction.getController(), targetEntity.getID());

		if (!m_pathToPosition.empty())
		{
			switchToState(eUnitState::Moving, map);
		}
		else if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(eUnitState::Moving, map);
		}
		else
		{
			assert(PathFinding::getInstance().isTargetInLineOfSight(*this, targetEntity, map));
			switchToState(eUnitState::AttackingTarget, map);
		}
	}
	else
	{
		if (m_pathToPosition.empty())
		{
			if (closestDestination != m_position)
			{
				m_pathToPosition.push_back(closestDestination);
				switchToState(eUnitState::Moving, map);
			}
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}
	}
}

void Unit::moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map,
	FactionHandler& factionHandler)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	if (PathFinding::getInstance().setUnitAttackPosition(*this, targetEntity, m_pathToPosition, map,
		m_owningFaction.getUnits(), factionHandler))
	{
		m_targetEntity.set(targetFaction.getController(), targetEntity.getID());

		if (!m_pathToPosition.empty())
		{
			switchToState(eUnitState::Moving, map);
		}
		else if(closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(eUnitState::Moving, map);
		}
		else
		{
			assert(PathFinding::getInstance().isTargetInLineOfSight(*this, targetEntity, map));
			switchToState(eUnitState::AttackingTarget, map);
		}
	}
	else
	{
		if (m_pathToPosition.empty())
		{
			if (closestDestination != m_position)
			{
				m_pathToPosition.push_back(closestDestination);
				switchToState(eUnitState::Moving, map);
			}
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

	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, adjacentPositions,
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

void Unit::update(float deltaTime, FactionHandler& factionHandler, const Map& map, const Timer& unitStateHandlerTimer)
{
	m_attackTimer.update(deltaTime);

	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		m_position = newPosition;
		m_AABB.update(m_position);

		if (m_position == m_pathToPosition.back())
		{
			switch (getEntityType())
			{
			case eEntityType::Unit:
				assert(Globals::isOnMiddlePosition(m_position));
				break;
			case eEntityType::Worker:
				break;
			default:
				assert(false);
			}
			
			m_pathToPosition.pop_back();

			if (getEntityType() == eEntityType::Unit && m_pathToPosition.empty())
			{
				GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
			}
		}
	}

	switch (m_currentState)
	{
	case eUnitState::Idle:
		assert(m_targetEntity.getID() == Globals::INVALID_ENTITY_ID);
		if (unitStateHandlerTimer.isExpired())
		{
			switch (getEntityType())
			{
			case eEntityType::Unit:
				for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
				{
					const Entity* targetEntity = opposingFaction.get().getEntity(m_position, UNIT_ATTACK_RANGE, true);
					if (targetEntity)
					{
						moveToAttackPosition(*targetEntity, opposingFaction, map, m_owningFaction);
					}
				}
				break;
			case eEntityType::Worker:
				break;
			default:
				assert(false);
			}
		}
		break;
	case eUnitState::Moving:
		if (m_targetEntity.getID() != Globals::INVALID_ENTITY_ID)
		{
			if (unitStateHandlerTimer.isExpired() &&
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
		if (unitStateHandlerTimer.isExpired())
		{
			for (const auto& opposingFaction : factionHandler.getOpposingFactions(m_owningFaction.getController()))
			{
				const Entity* targetEntity = opposingFaction.get().getEntity(m_position, UNIT_ATTACK_RANGE);
				if (targetEntity && PathFinding::getInstance().isTargetInLineOfSight(*this, *targetEntity, map))
				{
					moveToAttackPosition(*targetEntity, opposingFaction, map, m_owningFaction);
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
		if (unitStateHandlerTimer.isExpired())
		{
			const Entity* targetEntity = nullptr;
			if (factionHandler.isFactionActive(m_targetEntity.getFactionController()))
			{
				targetEntity = factionHandler.getFaction(m_targetEntity.getFactionController()).getEntity(m_targetEntity.getID());
				if (targetEntity)
				{
					const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
					moveToAttackPosition(*targetEntity, targetFaction, map, m_owningFaction);
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
		if (unitStateHandlerTimer.isExpired())
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
						!PathFinding::getInstance().isTargetInLineOfSight(*this, *targetEntity, map))
					{
						moveToAttackPosition(*targetEntity, targetFaction, map, m_owningFaction);
					}
					else if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
					{
						m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
					}
				}
			}
			else
			{
				switchToState(eUnitState::Idle, map);
			}
		}

		if (m_attackTimer.isExpired() && factionHandler.isFactionActive(m_targetEntity.getFactionController()))
		{
			const Faction& targetFaction = factionHandler.getFaction(m_targetEntity.getFactionController());
			const Entity* targetEntity = targetFaction.getEntity(m_targetEntity.getID());
			if (targetEntity && Globals::getSqrDistance(targetEntity->getPosition(), m_position) <= UNIT_ATTACK_RANGE * UNIT_ATTACK_RANGE)
			{
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createSpawnProjectile(m_owningFaction.getController(), getID(),
					targetFaction.getController(), targetEntity->getID(), DAMAGE, m_position, targetEntity->getPosition()));

				m_attackTimer.resetElaspedTime();
			}
		}
	
		break;
	}
}

void Unit::reduceHealth(const TakeDamageEvent& gameEvent, FactionHandler& factionHandler, const Map& map)
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
				moveToAttackPosition(*targetEntity, opposingFaction, map, m_owningFaction);
			}	
		}
	}
}

void Unit::switchToState(eUnitState newState, const Map& map, const Entity* targetEntity)
{
	if (getEntityType() == eEntityType::Unit && 
		(newState == eUnitState::Moving || newState == eUnitState::AttackMoving))
	{
		GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB, getID() });
	}

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
		m_attackTimer.resetElaspedTime();
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