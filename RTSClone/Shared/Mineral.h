#pragma once

#include "Entity.h"

class Mineral : public Entity
{
public:
#ifdef LEVEL_EDITOR
	Mineral();
#endif // LEVEL_EDITOR
#ifdef GAME
	Mineral(const glm::vec3& startingPosition);
#endif // GAME
	Mineral(Mineral&&) noexcept;
	Mineral& operator=(Mineral&&) noexcept;
	~Mineral();
};