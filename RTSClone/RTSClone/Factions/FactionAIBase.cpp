#include "FactionAIBase.h"
#include "Entities/Worker.h"
#include "Entities/Headquarters.h"
#include "Core/Base.h"
#include <assert.h>

FactionAIOccupiedMineral::FactionAIOccupiedMineral(Mineral& mineral)
	: mineral(&mineral)
{
}

//FactionAIBase
FactionAIBase::FactionAIBase(Headquarters& headquarters, HarvestLocation& harvest_location)
	: headquarters(&headquarters),
	harvest_location(&harvest_location)
{
	for (auto& mineral : harvest_location.minerals)
	{
		harvest_location_minerals.emplace_back(mineral);
	}
}

bool FactionAIBase::AddWorker(Worker& worker, const int id)
{
	auto spawner = std::find_if(all_entities.cbegin(), all_entities.cend(), [id](const auto& entity)
	{
		return entity->getID() == id;
	});
	if (spawner != all_entities.cend())
	{
		return false;
	}

	workers.push_back(&worker);
	return true;
}

//FactionAIBaseManager

void FactionAIBaseManager::RegisterBase(const HarvestLocation& harvest_location, const Headquarters& headquarters)
{
}

void FactionAIBaseManager::RegisterWorker(Worker& worker, const int spawner_id)
{
	for (auto& base : m_bases)
	{
		base.AddWorker(worker, spawner_id);
	}
}

