#include "AIOccupiedBases.h"
#include "Worker.h"
#include "Base.h"
#include "Headquarters.h"
#include "Globals.h"
#include "Turret.h"
#include "GameEventHandler.h"
#include "GameEvents.h"
#include <assert.h>

//AIOccupiedBase
AIOccupiedBase::AIOccupiedBase(const Base& base)
	: base(base),
	workers(),
	buildings(),
	turretCount(0),
	barracksCount(0),
	supplyDepotCount(0),
	laboratoryCount(0)
{}

bool AIOccupiedBase::isWorkerAdded(const Worker& worker) const
{
	auto iter = std::find_if(workers.cbegin(), workers.cend(), [&worker](const auto& i)
	{
		return worker.getID() == i.get().getID();
	});

	return iter != workers.cend();
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

//AIOccupiedBases
AIOccupiedBases::AIOccupiedBases(const BaseHandler& baseHandler)
	: m_bases()
{
	m_bases.reserve(baseHandler.getBases().size());
	for (const auto& base : baseHandler.getBases())
	{
		m_bases.emplace_back(base);
	}
}

const std::vector<AIOccupiedBase>& AIOccupiedBases::getBases() const
{
	return m_bases;
}

AIOccupiedBase* AIOccupiedBases::getBase(const glm::vec3& position)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&position](const auto& base)
	{
		return base.base.get().getCenteredPosition() == position;
	});
	
	return base != m_bases.end() ? &(*base) : nullptr;
}

const AIOccupiedBase* AIOccupiedBases::getBase(const Base& _base) const
{
	auto iter = std::find_if(m_bases.cbegin(), m_bases.cend(), [&_base](const auto& base)
	{
		return _base.getCenteredPosition() == base.base.get().getCenteredPosition();
	});

	return iter != m_bases.cend() ? &(*iter) : nullptr;
}

AIOccupiedBase* AIOccupiedBases::getBase(const Entity& entity)
{
	AIOccupiedBase* occupiedBase = nullptr;
	switch (entity.getEntityType())
	{
		case eEntityType::Headquarters:
		{
			AIOccupiedBase* base = getBase(entity.getPosition());
			assert(base);
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
			for (auto& base : m_bases)
			{
				const Entity* building = base.getBuilding(entity);
				if (building && building->getID() == entity.getID())
				{
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
	std::sort(m_bases.begin(), m_bases.end(), [&position](const auto& a, const auto& b)
		{ return Globals::getSqrDistance(a.base.get().getCenteredPosition(), position) < 
			Globals::getSqrDistance(b.base.get().getCenteredPosition(), position); });

	return m_bases;
}

void AIOccupiedBases::addWorker(Worker& worker, const Headquarters& headquarters)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&headquarters](const auto& base)
	{
		return base.base.get().getCenteredPosition() == headquarters.getPosition();
	});
	assert(base != m_bases.end());

	base->addWorker(worker);
}

void AIOccupiedBases::addWorker(Worker& worker, const Base& base)
{
	auto iter = std::find_if(m_bases.begin(), m_bases.end(), [&base](const auto& existingBase)
	{
		return existingBase.base.get().getCenteredPosition() == base.getCenteredPosition();
	});

	assert(iter != m_bases.end());
	iter->addWorker(worker);
}

void AIOccupiedBases::removeWorker(const Worker& worker)
{
	for (auto& base : m_bases)
	{
		for (auto iter = base.workers.begin(); iter != base.workers.end(); ++iter)
		{
			if (iter->get().getID() == worker.getID())
			{
				base.workers.erase(iter);
				return;
			}
		}
	}
}

void AIOccupiedBases::addBuilding(const Worker& worker, const Entity& building)
{
	AIOccupiedBase* occupiedBase = getBaseWithWorker(worker);
	assert(occupiedBase);
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
	AIOccupiedBase* occupiedBase = nullptr;
	for (auto& base : m_bases)
	{
		auto iter = std::find_if(base.buildings.begin(), base.buildings.end(), [&building](const auto& i)
		{
			return building.getID() == i.get().getID();
		});
		if (iter != base.buildings.end())
		{
			base.buildings.erase(iter);
			occupiedBase = &base;
		}
	}

	assert(occupiedBase);
	switch (building.getEntityType())
	{
	case eEntityType::Barracks:
		--occupiedBase->barracksCount;
		break;
	case eEntityType::SupplyDepot:
		--occupiedBase->supplyDepotCount;
		break;
	case eEntityType::Laboratory:
		--occupiedBase->laboratoryCount;
		break;
	case eEntityType::Turret:
		--occupiedBase->turretCount;
		break;
	case eEntityType::Headquarters:
		break;
	default:
		assert(false);
	}
}

AIOccupiedBase* AIOccupiedBases::getBaseWithWorker(const Worker& worker)
{
	int workerID = worker.getID();
	for (auto& base : m_bases)
	{
		auto iter = std::find_if(base.workers.cbegin(), base.workers.cend(), [workerID](const auto& worker)
		{
			return worker.get().getID() == workerID;
		});
		if (iter != base.workers.cend())
		{
			return &base;
		}
	}

	return nullptr;
}
