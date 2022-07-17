#pragma once

#include "Core/Mineral.h"
#include <vector>
#include "Graphics/Quad.h"

struct HarvestLocation
{
	HarvestLocation(const glm::vec3& position, std::vector<Mineral>&& minerals);

	glm::vec3 position{ 0.f };
	std::vector<Mineral> minerals{};
};

class HarvestLocationManager
{
public:
	HarvestLocationManager(std::vector<HarvestLocation>&& locations);

	const std::vector<HarvestLocation>& HarvestLocations() const;
	const HarvestLocation& ClosestHarvestLocation(const glm::vec3& position) const;
	const Mineral* MineralAtPosition(const glm::vec3& position) const;

	void Render(ShaderHandler& shader_handler) const;

private:
	std::vector<HarvestLocation> m_locations;
};