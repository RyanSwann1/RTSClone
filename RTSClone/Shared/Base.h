#pragma once

#include "Mineral.h"
#include <vector>
#ifdef LEVEL_EDITOR
#include "Quad.h"
#endif // LEVEL_EDITOR

struct Base
{
#ifdef GAME
	Base(const glm::vec3& position, std::vector<Mineral>&& minerals);
#endif // GAME
	Base(const glm::vec3& position);

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
	Quad quad;
#endif // LEVEL_EDITOR

	glm::vec3 position;
	std::vector<Mineral> minerals;
};