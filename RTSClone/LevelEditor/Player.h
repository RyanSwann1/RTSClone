#pragma once

#include "Entity.h"
#include "Globals.h"
#include "FactionController.h"
#include <array>
#include <string>
#include <ostream>

struct Player : private NonCopyable
{
	Player(eFactionController factionController);
	Player(eFactionController factionController, const glm::vec3& hqStartingPosition, const glm::vec3& startingMineralPosition);
	Player(Player&&) noexcept;
	Player& operator=(Player&&) noexcept;

	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	friend const std::ifstream& operator>>(std::ifstream& file, Player& player);
	friend std::ostream& operator<<(std::ostream& ostream, const Player& player);

	eFactionController controller;
	Entity HQ;
	std::array<Entity, Globals::MAX_MINERALS_PER_FACTION> minerals;
};