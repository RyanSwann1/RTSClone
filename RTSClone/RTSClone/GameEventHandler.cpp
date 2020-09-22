#include "GameEventHandler.h"
#include "Globals.h"
#include "Faction.h"
#include "ProjectileHandler.h"
#include "FactionHandler.h"
#include "Level.h"

GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::handleEvents(Level & level, const Map & map)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		default:
		case eGameEventType::Attack:
		case eGameEventType::RemovePlannedBuilding:
		case eGameEventType::RemoveAllWorkerPlannedBuildings:
		case eGameEventType::AddResources:
		case eGameEventType::SpawnProjectile:
		case eGameEventType::RevalidateMovementPaths:
		case eGameEventType::FactionEliminated:
		case eGameEventType::PlayerSpawnUnit:
		case eGameEventType::PlayerActivatePlannedBuilding:
		case eGameEventType::RepairEntity:
			level.handleEvent(gameEvent, map);
		}

		m_gameEvents.pop();
	}
}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}