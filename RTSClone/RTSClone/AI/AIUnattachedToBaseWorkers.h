#pragma once

#include <vector>
#include <functional>
#include "glm/glm.hpp"

class Worker;
class AIUnattachedToBaseWorkers
{
public:
	AIUnattachedToBaseWorkers();
	AIUnattachedToBaseWorkers(const AIUnattachedToBaseWorkers&) = delete;
	AIUnattachedToBaseWorkers& operator=(const AIUnattachedToBaseWorkers&) = delete;
	AIUnattachedToBaseWorkers(AIUnattachedToBaseWorkers&&) = delete;
	AIUnattachedToBaseWorkers& operator=(AIUnattachedToBaseWorkers&&) = delete;

	bool isEmpty() const;
	Worker& getClosestWorker(const glm::vec3& position);

	void addWorker(Worker& worker);
	void remove(const Worker& worker);

private:
	std::vector<std::reference_wrapper<Worker>> m_unattachedToBaseWorkers;
};