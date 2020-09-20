#include "GameEventHandler.h"
#include "Globals.h"
#include "Faction.h"
#include "ProjectileHandler.h"
#include "FactionHandler.h"
#include "Level.h"

GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}

void GameEventHandler::handleEvents(FactionHandler& factionHandler, ProjectileHandler& projectileHandler, const Map& map, Level& level)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		case eGameEventType::Attack:
			if (factionHandler.isFactionActive(gameEvent.targetFaction))
			{
				factionHandler.fgetFaction(gameEvent.targetFaction).handleEvent(gameEvent, map);
			}
			break;
		case eGameEventType::RemovePlannedBuilding:
		case eGameEventType::RemoveAllWorkerPlannedBuildings:
		case eGameEventType::AddResources:
			if (factionHandler.isFactionActive(gameEvent.senderFaction))
			{
				factionHandler.fgetFaction(gameEvent.senderFaction).handleEvent(gameEvent, map);
			}
			break;
		case eGameEventType::SpawnProjectile:
			projectileHandler.addProjectile(gameEvent);
			break;
		case eGameEventType::RevalidateMovementPaths:
			for (auto& faction : factionHandler.getFactions())
			{
				if (faction)
				{
					faction->handleEvent(gameEvent, map);
				}
			}
			break;
		case eGameEventType::FactionEliminated:
			level.handleEvent(gameEvent, map);
			break;
		case eGameEventType::SpawnUnit:
			level.handleEvent(gameEvent, map);
			break;
		default:
			assert(false);
		}

		m_gameEvents.pop();
	}
}