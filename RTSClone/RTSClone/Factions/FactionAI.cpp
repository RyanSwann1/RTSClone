#include "FactionAI.h"
#include "Model/AdjacentPositions.h"
#include "Graphics/ModelManager.h"
#include "Core/PathFinding.h"
#include "FactionHandler.h"
#include "Core/Level.h"
#include "Events/GameMessages.h"
#include "Events/GameMessenger.h"
#include "Core/Base.h"
#include <limits>
#include <algorithm>

//Levels
//Strategyt level - general - thgought about game state as a whole  where units are - lacing resources? Or attack enemy base - all high level
//Tactics level - Act on tactics from strategy layer - gain resources or attack enemy - 
//Team Level - made up of units - 

//IF paleyur attacks mid tactic - abondon plan - change tactic
//Each of tactic - in script
//Set of steps: move team 1 to front of enemy base - move team 2 back of enemy base

//Threat detection map - base A* off of the threat of the map - move across area of least threat

//Was tactic successful? If so do it more - Feedback on performance
//Keeping track of wher enemy atacks your base, building turrets in appriopriate places
//Overview of  how AI will work generally.


FactionAI_NEW::FactionAI_NEW(eFactionController factionController, const glm::vec3& hqStartingPosition, 
	int startingResources, int startingPopulationCap, AIConstants::eBehaviour behaviour,
	const HarvestLocationManager& harvest_location_manager)
	: Faction(factionController, hqStartingPosition, startingResources, startingPopulationCap),
	m_harvest_location_manager(harvest_location_manager)
{
	m_action_queue.emplace(FactionAIAction::BuildWorker);

	for (auto& headquarters : m_headquarters)
	{
		m_occupied_base_manager.RegisterBase(
			m_harvest_location_manager.ClosestHarvestLocation(headquarters.getPosition()), headquarters);
	}
}

void FactionAI_NEW::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
	Faction::handleEvent(gameEvent, map, factionHandler);

	switch (gameEvent.type)
	{
	case eGameEventType::EntityIdle:
	{
		auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [entityID = gameEvent.data.entityIdle.entityID](const auto& entity)
		{
			return entity->getID() == entityID;
		});
		if (entity != m_allEntities.cend())
		{
			switch ((*entity)->getEntityType())
			{
			case eEntityType::Unit:
				break;
			case eEntityType::Worker:
				OnWorkerIdle(static_cast<Worker&>(*(*entity)), map);
				break;
			}
		}
	}
	}
}

void FactionAI_NEW::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
	Faction::update(deltaTime, map, factionHandler);
	
	if (!m_action_queue.empty())
	{
		switch (m_action_queue.front())
		{
		case FactionAIAction::BuildWorker:
			if (RequestWorkerCreation())
			{
				m_action_queue.pop();
			}
			break;
		default:
			assert(false);
		}
	}
}

bool FactionAI_NEW::RequestWorkerCreation()
{
	if (m_headquarters.empty() 
		|| !IsEntityCreatable(eEntityType::Worker))
	{
		return false;
	}
	
	return m_headquarters.back().AddEntityToSpawnQueue(*this);
}

void FactionAI_NEW::OnWorkerIdle(Worker& worker, const Map& map)
{
	const HarvestLocation& closest_harvest_location{ m_harvest_location_manager.ClosestHarvestLocation(worker.getPosition()) };
	worker.Harvest(closest_harvest_location.minerals[0], map);
}