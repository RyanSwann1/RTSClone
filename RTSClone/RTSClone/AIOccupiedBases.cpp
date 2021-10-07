#include "AIOccupiedBases.h"
#include "Worker.h"
#include "Base.h"
#include "Headquarters.h"
#include "Globals.h"
#include "Turret.h"
#include "GameEventHandler.h"
#include "GameEvents.h"
#include "FactionAI.h"
#include <assert.h>

//AIOccupiedBase
AIOccupiedBase::AIOccupiedBase(const Base& base)
	: base(base),
	actionQueue(),
	actionPriorityQueue(),
	workers(),
	buildings(),
	turretCount(0),
	barracksCount(0),
	supplyDepotCount(0),
	laboratoryCount(0)
{}

eFactionController AIOccupiedBase::getFactionController() const
{
	return base.get().owningFactionController;
}

bool AIOccupiedBase::isWorkerAdded(const Worker& worker) const
{
	auto iter = std::find_if(workers.cbegin(), workers.cend(), [&worker](const auto& i)
	{
		return worker.getID() == i.get().getID();
	});

	return iter != workers.cend();
}

int AIOccupiedBase::getQueuedAIActionTypeCount(eAIActionType actionType) const
{
	int count = actionPriorityQueue.getActionTypeCount(actionType);
	for (auto action : actionQueue)
	{
		if (action.actionType == actionType)
		{
			++count;
		}
	}

	return count;
}

int AIOccupiedBase::getWorkerBuildQueueCount(eEntityType entityType) const
{
	int count = 0;
	for (const auto& worker : workers)
	{
		for (const auto& buildingInQueue : worker.get().getBuildingCommands())
		{
			if (buildingInQueue.entityType == entityType)
			{
				++count;
			}
		}
	}

	return count;
}

int AIOccupiedBase::getSpawnedEntityCount(eEntityType entityType) const
{
	switch (entityType)
	{
		case eEntityType::Worker:
			return static_cast<int>(workers.size());
		case eEntityType::SupplyDepot:
			return supplyDepotCount;
		case eEntityType::Barracks:
			return barracksCount;
		case eEntityType::Turret:
			return turretCount;
		case eEntityType::Laboratory:
			return laboratoryCount;
		case eEntityType::Unit:
		case eEntityType::Headquarters:
			break;
		default:
			assert(false);
	}

	return 0;
}

const Entity* AIOccupiedBase::getBuilding(const Entity& building) const
{
	int buildingID = building.getID();
	auto iter = std::find_if(buildings.cbegin(), buildings.cend(), [buildingID](const auto& building)
	{
		return building.get().getID() == buildingID;
	});

	return iter != buildings.cend() ? &(*iter).get() : nullptr;
}

Entity* AIOccupiedBase::getBuilding(eEntityType entityType) const
{
	auto building = std::find_if(buildings.begin(), buildings.end(), [entityType](const auto& building)
	{
		return building.get().getEntityType() == entityType;
	});
	return building != buildings.end() ? &(*building).get() : nullptr;
}

void AIOccupiedBase::addWorker(Worker& worker)
{
	assert(std::find_if(workers.cbegin(), workers.cend(), [&worker](const auto& i)
	{
		return i.get().getID() == worker.getID();
	}) == workers.cend());
	workers.emplace_back(worker);
}

void AIOccupiedBase::removeWorker(const Worker& worker)
{
	auto iter = std::find_if(workers.begin(), workers.end(), [&worker](const auto& i)
	{
		return i.get().getID() == worker.getID();
	});
	assert(iter != workers.end());
	workers.erase(iter);
}

const Entity* AIOccupiedBase::removeBuilding(const Entity& building)
{
	const Entity* buildingRemoved = nullptr;
	auto iter = std::find_if(buildings.begin(), buildings.end(), [&building](const auto& i)
	{
		return building.getID() == i.get().getID();
	});
	if (iter != buildings.end())
	{
		buildingRemoved = &(*iter).get();
		buildings.erase(iter);
	}

	return buildingRemoved;
}

//AIOccupiedBases
AIOccupiedBases::AIOccupiedBases(const BaseHandler& baseHandler, eFactionController owningFaction)
	: bases(),
	owningFaction(owningFaction)
{
	bases.reserve(baseHandler.getBases().size());
}

AIOccupiedBase* AIOccupiedBases::getBase(const glm::vec3& position)
{
	auto base = std::find_if(bases.begin(), bases.end(), [&position](const auto& base)
	{
		return base.base.get().getCenteredPosition() == position;
	});
	
	return base != bases.end() ? &(*base) : nullptr;
}

AIOccupiedBase* AIOccupiedBases::getBase(const Entity& entity)
{
	AIOccupiedBase* occupiedBase = nullptr;
	switch (entity.getEntityType())
	{
		case eEntityType::Headquarters:
		{
			AIOccupiedBase* base = getBase(entity.getPosition());
			assert(base && base->base.get().owningFactionController == owningFaction);
			if (base)
			{
				occupiedBase = base;
			}
		}
		break;
		case eEntityType::Worker:
		occupiedBase = getBaseWithWorker(static_cast<const Worker&>(entity));
		break;
		case eEntityType::SupplyDepot:
		case eEntityType::Barracks:
		case eEntityType::Turret:
		case eEntityType::Laboratory:
			for (auto& base : bases)
			{
				const Entity* building = base.getBuilding(entity);
				if (building && building->getID() == entity.getID())
				{
					assert(base.base.get().owningFactionController == owningFaction);
					occupiedBase = &base;
					break;
				}
			}
		break;
		case eEntityType::Unit:
		break;
		default:
			assert(false);
	}

	return occupiedBase;
}

const std::vector<AIOccupiedBase>& AIOccupiedBases::getSortedBases(const glm::vec3& position)
{
	std::sort(bases.begin(), bases.end(), [&position](const auto& a, const auto& b)
		{ return Globals::getSqrDistance(a.base.get().getCenteredPosition(), position) < 
			Globals::getSqrDistance(b.base.get().getCenteredPosition(), position); });

	return bases;
}

void AIOccupiedBases::addWorker(Worker& worker, const Headquarters& headquarters)
{
	auto base = std::find_if(bases.begin(), bases.end(), [&headquarters](const auto& base)
	{
		return base.base.get().getCenteredPosition() == headquarters.getPosition();
	});

	assert(base != bases.end() && (*base).base.get().owningFactionController == owningFaction);
	(*base).addWorker(worker);
}

void AIOccupiedBases::addWorker(Worker& worker, const Base& base)
{
	auto iter = std::find_if(bases.begin(), bases.end(), [&base](const auto& existingBase)
	{
		return existingBase.base.get().getCenteredPosition() == base.getCenteredPosition();
	});
	
	assert(iter != bases.end() && base.owningFactionController == owningFaction);
	iter->addWorker(worker);
}

void AIOccupiedBases::removeWorker(const Worker& worker)
{
	for (auto& base : bases)
	{
		for (auto iter = base.workers.begin(); iter != base.workers.end(); ++iter)
		{
			if (iter->get().getID() == worker.getID())
			{
				assert(base.base.get().owningFactionController == owningFaction);
				base.workers.erase(iter);
				return;
			}
		}
	}
}

AIOccupiedBase& AIOccupiedBases::addBase(const Base& base)
{
	assert(base.owningFactionController == owningFaction);
	bases.emplace_back(base);
	return bases.back();
}

std::vector<std::reference_wrapper<Worker>> AIOccupiedBases::removeBase(const Base& base)
{
	auto iter = std::find_if(bases.begin(), bases.end(), [&base](const auto& existingBase)
	{
		return existingBase.base.get().position == base.position;
	});
	assert(iter != bases.end());
	std::vector<std::reference_wrapper<Worker>> workers = std::move(iter->workers);
	bases.erase(iter);
	return workers;
}

void AIOccupiedBases::addBuilding(const Worker& worker, Entity& building)
{
	AIOccupiedBase* occupiedBase = getBaseWithWorker(worker);
	assert(occupiedBase && occupiedBase->base.get().owningFactionController == owningFaction);
	if (occupiedBase)
	{
		assert(std::find_if(occupiedBase->buildings.cbegin(), occupiedBase->buildings.cend(), [&building](const auto& i)
		{
			return i.get().getID() == building.getID();
		}) == occupiedBase->buildings.cend());

		switch (building.getEntityType())
		{
			case eEntityType::Barracks:
			++occupiedBase->barracksCount;
			occupiedBase->buildings.emplace_back(building);
			break;
			case eEntityType::SupplyDepot:
			++occupiedBase->supplyDepotCount;
			occupiedBase->buildings.emplace_back(building);
			break;
			case eEntityType::Laboratory:
			++occupiedBase->laboratoryCount;
			occupiedBase->buildings.emplace_back(building);
			break;
			case eEntityType::Turret:
			++occupiedBase->turretCount;
			occupiedBase->buildings.emplace_back(building);
			break;
			case eEntityType::Headquarters:
			break;
			default:
			assert(false);
		}
	}
}

void AIOccupiedBases::removeBuilding(const Entity& building)
{
	if (building.getEntityType() == eEntityType::Headquarters)
	{
		return;
	}

	const Entity* buildingRemoved = nullptr;
	for (auto& base : bases)
	{
		buildingRemoved = base.removeBuilding(building);
		if (buildingRemoved)
		{
			assert(base.base.get().owningFactionController == owningFaction);
			switch (building.getEntityType())
			{
			case eEntityType::Barracks:
				--base.barracksCount;
				break;
			case eEntityType::SupplyDepot:
				--base.supplyDepotCount;
				break;
			case eEntityType::Laboratory:
				--base.laboratoryCount;
				break;
			case eEntityType::Turret:
				--base.turretCount;
				break;
			default:
				assert(false);
			}

			break;
		}
	}

	assert(buildingRemoved);
}

AIOccupiedBase* AIOccupiedBases::getBaseWithWorker(const Worker& worker)
{
	int workerID = worker.getID();
	for (auto& base : bases)
	{
		auto iter = std::find_if(base.workers.cbegin(), base.workers.cend(), [workerID](const auto& worker)
		{
			return worker.get().getID() == workerID;
		});
		if (iter != base.workers.cend())
		{
			assert(base.base.get().owningFactionController == owningFaction);
			return &base;
		}
	}

	return nullptr;
}
