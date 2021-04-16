#include "AIOccupiedBases.h"
#include "Worker.h"
#include "Base.h"
#include "Headquarters.h"
#include "Globals.h"
#include "Turret.h"
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
}

AIOccupiedBase& AIOccupiedBases::getBase(const Headquarters& headquarters)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&headquarters](const auto& base)
	{
		return base.base.get().getCenteredPosition() == headquarters.getPosition();
	});
	
	assert(base != m_bases.end());
	return (*base);
}

const AIOccupiedBase& AIOccupiedBases::getBase(const Base& _base) const
{
	auto iter = std::find_if(m_bases.cbegin(), m_bases.cend(), [&_base](const auto& base)
	{
		return _base.getCenteredPosition() == base.base.get().getCenteredPosition();// headquarters.getPosition();
	});

	assert(iter != m_bases.cend());
	return (*iter);
}

AIOccupiedBase* AIOccupiedBases::getBase(const Worker& worker)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&worker](const auto& base)
	{
		return base.isWorkerAdded(worker);
	});

	return base != m_bases.end() ? &(*base) : nullptr;
}

const std::vector<AIOccupiedBase>& AIOccupiedBases::getSortedBases(const glm::vec3& position)
{
	std::sort(m_bases.begin(), m_bases.end(), [&position](const auto& a, const auto& b)
		{ return Globals::getSqrDistance(a.base.get().getCenteredPosition(), position) < 
			Globals::getSqrDistance(b.base.get().getCenteredPosition(), position); });

	return m_bases;
}

void AIOccupiedBases::addBase(const Base& base)
{
	assert(std::find_if(m_bases.cbegin(), m_bases.cend(), [&base](const auto& existingBase)
	{
		return existingBase.base.get().getCenteredPosition() == base.getCenteredPosition();
	}) == m_bases.cend());
	m_bases.emplace_back(base);
}

void AIOccupiedBases::removeBase(const Base& base)
{
	auto iter = std::find_if(m_bases.begin(), m_bases.end(), [&base](const auto& existingBase)
	{
		return existingBase.base.get().getCenteredPosition() == base.getCenteredPosition();
	});
	assert(iter != m_bases.end());
	m_bases.erase(iter);
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
	AIOccupiedBase* occupiedBase = getBase(worker);
	assert(occupiedBase);
	if (occupiedBase)
	{
		assert(std::find_if(occupiedBase->buildings.cbegin(), occupiedBase->buildings.cend(), [&building](const auto& i)
		{
			return i.get().getID() == building.getID();
		}) == occupiedBase->buildings.cend());
		occupiedBase->buildings.emplace_back(building);

		switch (building.getEntityType())
		{
			case eEntityType::Barracks:
			++occupiedBase->barracksCount;
			break;
			case eEntityType::SupplyDepot:
			++occupiedBase->supplyDepotCount;
			break;
			case eEntityType::Laboratory:
			++occupiedBase->laboratoryCount;
			break;
			case eEntityType::Turret:
			++occupiedBase->turretCount;
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
	default:
		assert(false);
	}
}