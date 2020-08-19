#pragma once

#include "Entity.h"

class Mineral : public Entity
{
public:
	Mineral(int ID, const glm::vec3& startingPosition);
	Mineral(Mineral&&) noexcept;
	Mineral& operator=(Mineral&&) noexcept;
	~Mineral();
};