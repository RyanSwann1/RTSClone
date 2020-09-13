#pragma once

#include "Entity.h"
#include "Globals.h"
#include "PlayerType.h"
#include <array>
#include <string>
#include <ostream>

struct Player
{
	Player(ePlayerType playerType, const glm::vec3& startingHQPosition, const glm::vec3& startingMineralPosition);

	void render(ShaderHandler& shaderHandler) const;
	
	friend std::ostream& operator<<(std::ostream& ostream, const Player& player);

	const ePlayerType type;
	Entity HQ;
	std::array<Entity, Globals::MAX_MINERALS_PER_FACTION> minerals;
};