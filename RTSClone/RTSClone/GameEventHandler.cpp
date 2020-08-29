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

void GameEventHandler::handleEvents(FactionPlayer& player, FactionAI& playerAI, ProjectileHandler& projectileHandler)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		case eGameEventType::Attack:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				playerAI.handleEvent(gameEvent);
			}
			else if(gameEvent.senderFaction == eFactionName::AI)
			{
				player.handleEvent(gameEvent);
			}
			break;
		case eGameEventType::RemovePlannedBuilding:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				player.handleEvent(gameEvent);
			}
			else if (gameEvent.senderFaction == eFactionName::AI)
			{
				playerAI.handleEvent(gameEvent);
			}
			break;
		case eGameEventType::AddResources:
			if (gameEvent.senderFaction == eFactionName::Player)
			{
				player.handleEvent(gameEvent);
			}
			else if (gameEvent.senderFaction == eFactionName::AI)
			{
				playerAI.handleEvent(gameEvent);
			}
			break;
		case eGameEventType::SpawnProjectile:
			projectileHandler.addProjectile(gameEvent);
			break;
		default:
			assert(false);
		}

		m_gameEvents.pop();
	}
}