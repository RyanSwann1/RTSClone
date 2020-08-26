#include "GameEventHandler.h"
#include "Globals.h"
#include "FactionPlayer.h"
#include "FactionAI.h"

//GameEvent
GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID)
{}

//GameEventHandler
GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}

void GameEventHandler::handleEvents(FactionPlayer& player, FactionAI& playerAI)
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
			else
			{
				assert(false);
			}
			break;
		default:
			assert(false);
		}

		m_gameEvents.pop();
	}
}