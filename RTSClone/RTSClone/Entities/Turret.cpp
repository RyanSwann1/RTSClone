#include "Turret.h"
#include "Graphics/ModelManager.h"
#include "Factions/FactionHandler.h"
#include "Core/Globals.h"
#include "Core/PathFinding.h"
#include "Events/GameEvents.h"
#include "Events/GameMessages.h"
#include "Events/GameMessenger.h"
#include "Core/Level.h"

namespace
{
	const int TURRET_STARTING_HEALTH = 5;
	const int TURRET_DAMAGE = 2;
	const float TURRET_ATTACK_RANGE = Globals::NODE_SIZE * 7.0f;
	const float TIME_BETWEEN_ATTACK = 2.25f;
	const float TIME_BETWEEN_IDLE_CHECK = 1.0f;
}

Turret::Turret(const Position& position, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(TURRET_MODEL_NAME), position,
		eEntityType::Turret, TURRET_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_stateHandlerTimer(0.2f, true),
	m_attackTimer(TIME_BETWEEN_ATTACK, true)
{
	broadcast<GameMessages::AddAABBToMap>({ m_AABB });
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

Turret::~Turret()
{
	if (getID() != UniqueID::INVALID_ID)
	{
		broadcast<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

bool Turret::is_group_selectable() const
{
	return false;
}

void Turret::update(float deltaTime, FactionHandler& factionHandler, const Map& map)
{
	Entity::update(deltaTime);
	m_stateHandlerTimer.update(deltaTime);
	m_attackTimer.update(deltaTime);

	if (m_target)
	{
		const Faction* opposingFaction = factionHandler.getFaction(m_target->controller);
		if (m_stateHandlerTimer.isExpired()
			&& opposingFaction)
		{
			const Entity* targetEntity = opposingFaction->get_entity(m_target->ID);
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position.Get()) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFinding::getInstance().isTargetInLineOfSight(m_position.Get(), *targetEntity, map, m_AABB))
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position.Get(), 270.0f);
			}
			else
			{
				m_target.reset();
			}
		}

		if (opposingFaction
			&& m_attackTimer.isExpired()
			&& factionHandler.isFactionActive(m_target->controller))
		{
			const Entity* targetEntity = opposingFaction->get_entity(m_target->ID);
			if (targetEntity &&
				Globals::getSqrDistance(targetEntity->getPosition(), m_position.Get()) <= TURRET_ATTACK_RANGE * TURRET_ATTACK_RANGE &&
				PathFinding::getInstance().isTargetInLineOfSight(m_position.Get(), *targetEntity, map, m_AABB))
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position.Get(), 270.0f);
				Level::add_event(GameEvent::create<SpawnProjectileEvent>({ m_owningFaction.get().getController(), getID(),
					getEntityType(), opposingFaction->getController(), targetEntity->getID(), targetEntity->getEntityType(),
					TURRET_DAMAGE, m_position.Get(), targetEntity->getPosition() }));

				m_attackTimer.resetElaspedTime();
			}
		}
	}
	else
	{
		if (m_stateHandlerTimer.isExpired())
		{
			for (const Faction* opposingFaction : factionHandler.GetOpposingFactions(m_owningFaction.get().getController()))
			{
				if (const Entity* targetEntity = opposingFaction->getEntity(m_position.Get(), TURRET_ATTACK_RANGE))
				{
					m_target = { opposingFaction->getController(), targetEntity->getID() };
					m_attackTimer.resetElaspedTime();
					break;
				}
			}
		}
	}

	if (m_stateHandlerTimer.isExpired())
	{
		m_stateHandlerTimer.resetElaspedTime();
	}
}