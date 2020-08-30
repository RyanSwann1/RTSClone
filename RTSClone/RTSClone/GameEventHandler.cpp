#include "GameEventHandler.h"
#include "Globals.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "ProjectileHandler.h"

GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}

void GameEventHandler::handleEvents(FactionPlayer& player, FactionAI& playerAI, ProjectileHandler& projectileHandler, const Map& map)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		case eGameEventType::Attack:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				playerAI.handleEvent(gameEvent, map);
			}
			else if(gameEvent.senderFaction == eFactionName::AI)
			{
				player.handleEvent(gameEvent, map);
			}
			break;
		case eGameEventType::RemovePlannedBuilding:
		case eGameEventType::RemoveAllWorkerPlannedBuildings:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				player.handleEvent(gameEvent, map);
			}
			else if (gameEvent.senderFaction == eFactionName::AI)
			{
				playerAI.handleEvent(gameEvent, map);
			}
			break;
		case eGameEventType::AddResources:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				player.handleEvent(gameEvent, map);
			}
			else if (gameEvent.senderFaction == eFactionName::AI)
			{
				playerAI.handleEvent(gameEvent, map);
			}
			break;
		case eGameEventType::SpawnProjectile:
			projectileHandler.addProjectile(gameEvent);
			break;
		case eGameEventType::RevalidateMovementPaths:
			player.handleEvent(gameEvent, map);
			playerAI.handleEvent(gameEvent, map);
			break;
		default:
			assert(false);
		}

		m_gameEvents.pop();
	}
}