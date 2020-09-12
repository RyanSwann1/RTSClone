#pragma once

#include "Entity.h"
#include "Globals.h"
#include <array>

struct Player
{
	Player(const glm::vec3& startingHQPosition, const glm::vec3& startingMineralPosition);

	void render(ShaderHandler& shaderHandler) const;

	std::array<Entity, Globals::MAX_MINERALS_PER_FACTION> m_minerals;
	Entity m_HQ;
};