#include "Core/Base.h"
#include "Core/Globals.h"
#include "../RTSClone/Events/GameEvents.h" // This really needs to be fixed..
#ifdef GAME
#include "Factions/Faction.h"
#include <limits>
#endif // GAME

namespace
{
	const glm::vec3 MAIN_BASE_QUAD_COLOR = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 MAIN_BASE_QUAD_SIZE = { Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE };
#ifdef GAME
	const float QUAD_OPACITY = 0.25f;
#endif // GAME
}

namespace
{
	std::array<const Mineral*, Globals::MAX_MINERALS> getClosestMinerals(const std::vector<Mineral>& minerals, const glm::vec3& position)
	{
		assert(static_cast<int>(minerals.size()) == Globals::MAX_MINERALS);
		std::array<const Mineral*, Globals::MAX_MINERALS> sortedMinerals;
		for (int i = 0; i < static_cast<int>(minerals.size()); ++i)
		{
			sortedMinerals[i] = &minerals[i];
		}

		std::sort(sortedMinerals.begin(), sortedMinerals.end(), [&position](const auto& mineralA, const auto& mineralB)
		{
			assert(mineralA && mineralB);
			return Globals::getSqrDistance(mineralA->getPosition(), position) <
				Globals::getSqrDistance(mineralB->getPosition(), position);
		});

		return sortedMinerals;
	}
}

HarvestLocationManager::HarvestLocationManager(std::vector<HarvestLocation>&& locations)
	: m_locations(std::move(locations))
{}

const std::vector<HarvestLocation>& HarvestLocationManager::HarvestLocations() const
{
	return m_locations;
}

const HarvestLocation& HarvestLocationManager::ClosestHarvestLocation(const glm::vec3& position) const
{
	const HarvestLocation* closest_harvest_location{ nullptr };
	float distance = std::numeric_limits<float>::max();
	for (auto& harvest_location : m_locations)
	{
		const float new_distance{ glm::distance(position, harvest_location.position) };
		if (new_distance < distance)
		{
			closest_harvest_location = &harvest_location;
			distance = new_distance;
		}
	}

	return *closest_harvest_location;
}

const Mineral* HarvestLocationManager::MineralAtPosition(const glm::vec3& position) const
{
	for (const auto& location : m_locations)
	{
		for (const auto& mineral : location.minerals)
		{
			if (mineral.getAABB().contains(position))
			{
				return &mineral;
			}
		}
	}

	return nullptr;
}

void HarvestLocationManager::Render(ShaderHandler& shader_handler) const
{
	for (const auto& location : m_locations)
	{
		for (const auto& mineral : location.minerals)
		{
			mineral.render(shader_handler);
		}
	}
}

HarvestLocation::HarvestLocation(const glm::vec3& position, std::vector<Mineral>&& minerals)
	: position(position),
	minerals(std::move(minerals))
{}