#pragma once

#include "glm/glm.hpp"
#include <queue>
#include <vector>
#ifdef RENDER_PATHING
#include "Graphics/Mesh.h"
#endif // RENDER_PATHING

struct Movement
{
	bool IsMovableAfterAddingDestination(const bool add_to_destinations, const glm::vec3& position);

	std::vector<glm::vec3> path;
	std::queue<glm::vec3> destinations;
#ifdef RENDER_PATHING
	Mesh pathMesh = {};
#endif // RENDER_PATHING
};