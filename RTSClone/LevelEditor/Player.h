#pragma once

#include "Globals.h"
#include "FactionController.h"
#include "GameObject.h"
#include <array>
#include <string>
#include <ostream>

struct Player : private NonCopyable, private NonMovable
{
	Player(eFactionController factionController);
	Player(eFactionController factionController, const glm::vec3& hqStartingPosition, const glm::vec3& startingMineralPosition);

	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	friend const std::ifstream& operator>>(std::ifstream& file, Player& player);
	friend std::ostream& operator<<(std::ostream& ostream, const Player& player);

	eFactionController controller;
	GameObject HQ;
	std::vector<GameObject> minerals;
};