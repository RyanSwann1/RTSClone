#include "GameEventHandler.h"
#include "Globals.h"
#include "Faction.h"
#include "ProjectileHandler.h"
#include "FactionHandler.h"

GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}

void GameEventHandler::handleEvents(FactionHandler& factionHandler, ProjectileHandler& projectileHandler, const Map& map)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		case eGameEventType::Attack:
			factionHandler.fgetFaction(gameEvent.targetFaction).handleEvent(gameEvent, map);
			break;
		case eGameEventType::RemovePlannedBuilding:
		case eGameEventType::RemoveAllWorkerPlannedBuildings:
		case eGameEventType::AddResources:
			factionHandler.fgetFaction(gameEvent.senderFaction).handleEvent(gameEvent, map);
			break;
		case eGameEventType::SpawnProjectile:
			projectileHandler.addProjectile(gameEvent);
			break;
		case eGameEventType::RevalidateMovementPaths:
			for (auto& faction : factionHandler.getFactions())
			{
				faction->handleEvent(gameEvent, map);
			}
			break;
		default:
			assert(false);
		}

		m_gameEvents.pop();
	}
}