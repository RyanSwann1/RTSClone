#include "GameEventHandler.h"
#include "Globals.h"
#include "Faction.h"
#include "ProjectileHandler.h"

GameEventHandler::GameEventHandler()
	: m_gameEvents()
{}

void GameEventHandler::addEvent(const GameEvent& gameEvent)
{
	m_gameEvents.push(gameEvent);
}

void GameEventHandler::handleEvents(std::vector<std::unique_ptr<Faction>>& factions, ProjectileHandler& projectileHandler, const Map& map)
{
	while (!m_gameEvents.empty())
	{
		const GameEvent& gameEvent = m_gameEvents.front();
		switch (gameEvent.type)
		{
		case eGameEventType::Attack:
		{
			auto faction = std::find_if(factions.begin(), factions.end(), [&gameEvent](const auto& faction)
			{
				return faction->getController() == gameEvent.targetFaction;
			});
			assert(faction != factions.end());

			faction->get()->handleEvent(gameEvent, map);
		}
			break;
		case eGameEventType::RemovePlannedBuilding:
		case eGameEventType::RemoveAllWorkerPlannedBuildings:
		{
			auto faction = std::find_if(factions.begin(), factions.end(), [&gameEvent](const auto& faction)
			{
				return faction->getController() == gameEvent.targetFaction;
			});
			assert(faction != factions.end());

			faction->get()->handleEvent(gameEvent, map);
		}
			break;
		case eGameEventType::AddResources:
		{
			auto faction = std::find_if(factions.begin(), factions.end(), [&gameEvent](const auto& faction)
			{
				return faction->getController() == gameEvent.senderFaction;
			});
			assert(faction != factions.end());

			faction->get()->handleEvent(gameEvent, map);
		}
			break;
		case eGameEventType::SpawnProjectile:
			projectileHandler.addProjectile(gameEvent);
			break;
		case eGameEventType::RevalidateMovementPaths:
			for (auto& faction : factions)
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