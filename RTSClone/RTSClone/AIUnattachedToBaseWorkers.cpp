#include "AIUnattachedToBaseWorkers.h"
#include <algorithm>
#include "Worker.h"
#include "Globals.h"

AIUnattachedToBaseWorkers::AIUnattachedToBaseWorkers()
	: m_unattachedToBaseWorkers() {}

bool AIUnattachedToBaseWorkers::isEmpty() const
{
	return m_unattachedToBaseWorkers.empty();
}

Worker& AIUnattachedToBaseWorkers::getClosestWorker(const glm::vec3& position)
{
	assert(!m_unattachedToBaseWorkers.empty());
	std::sort(m_unattachedToBaseWorkers.begin(), m_unattachedToBaseWorkers.end(), [&position](const auto& a, const auto& b)
	{
		return Globals::getSqrDistance(position, a.get().getPosition()) > Globals::getSqrDistance(position, b.get().getPosition());
	});
	Worker& worker = m_unattachedToBaseWorkers.back();
	m_unattachedToBaseWorkers.pop_back();
	return worker;
}

void AIUnattachedToBaseWorkers::addWorker(Worker& _worker)
{
	if (std::find_if(m_unattachedToBaseWorkers.cbegin(), m_unattachedToBaseWorkers.cend(), [&_worker](const auto& worker)
	{
		return _worker.getID() == worker.get().getID();
	}) == m_unattachedToBaseWorkers.cend())
	{
		m_unattachedToBaseWorkers.emplace_back(_worker);
	}
}

void AIUnattachedToBaseWorkers::remove(const Worker& _worker)
{
	auto iter = std::find_if(m_unattachedToBaseWorkers.begin(), m_unattachedToBaseWorkers.end(), [&_worker](const auto& worker)
	{
		return _worker.getID() == worker.get().getID();
	});
	assert(iter != m_unattachedToBaseWorkers.end());
	m_unattachedToBaseWorkers.erase(iter);
}