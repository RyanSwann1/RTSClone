#pragma once

#include <vector>

class Mineral;
class Worker;
struct FactionAIOccupiedMineral
{
	FactionAIOccupiedMineral(Mineral& mineral);

	Mineral* mineral{ nullptr };
	std::vector<Worker*> workers{};
};

struct HarvestLocation;
class Headquarters;
class Entity;
struct FactionAIBase
{
	FactionAIBase(Headquarters& headquarters, HarvestLocation& harvest_location);

	bool AddWorker(Worker& worker, const int id);

	std::vector<Entity*> all_entities{};
	Headquarters* headquarters{ nullptr };
	std::vector<Worker*> workers{};
	HarvestLocation* harvest_location{ nullptr };
	std::vector<FactionAIOccupiedMineral> harvest_location_minerals{};
};

class FactionAIBaseManager
{
public:
	FactionAIBaseManager() = default;

	void RegisterBase(const HarvestLocation& harvest_location, const Headquarters& headquarters);
	void RegisterWorker(Worker& worker, const int spawner_id);



private:
	std::vector<FactionAIBase> m_bases{};
};