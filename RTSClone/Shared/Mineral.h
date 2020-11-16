#pragma once

#include "Entity.h"

class Mineral : public Entity
{
public:
#ifdef LEVEL_EDITOR
	Mineral(const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
#endif // LEVEL_EDITOR
#ifdef GAME
	Mineral(const glm::vec3& startingPosition);
#endif // GAME
	Mineral(Mineral&&) noexcept;
	Mineral& operator=(Mineral&&) noexcept;
	~Mineral();
};