#include "AIOccupiedBases.h"
#include "Worker.h"
#include "Base.h"
#include "Headquarters.h"
#include "Globals.h"
#include <assert.h>

//AIOccupiedBase
AIOccupiedBase::AIOccupiedBase(const Base& base)
	: base(base),
	workers()
{}

void AIOccupiedBase::addWorker(const Worker& worker)
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

void AIOccupiedBases::addWorker(const Worker& worker, const Headquarters& headquarters)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&headquarters](const auto& base)
	{
		return base.base.get().getCenteredPosition() == headquarters.getPosition();
	});
	assert(base != m_bases.end());

	base->addWorker(worker);
}

void AIOccupiedBases::addWorker(const Worker& worker, const Base& base)
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