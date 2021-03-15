#pragma once

#include "AABB.h"
#ifdef GAME
#include "ActiveStatus.h"
#endif // GAME
#include <functional>

struct Model;
class ShaderHandler;
class Mineral
{
public:
#ifdef LEVEL_EDITOR
	Mineral();
#endif // LEVEL_EDITOR
	Mineral(const glm::vec3& startingPosition);
	Mineral(const Mineral&) = delete;
	Mineral& operator=(const Mineral&) = delete;
	Mineral(Mineral&&) = default;
	Mineral& operator=(Mineral&&) = default;
#ifdef GAME
	~Mineral();
#endif // GAME

	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
#endif // LEVEL_EDITOR
	void render(ShaderHandler& shaderHandler) const;

private:
#ifdef GAME
	ActiveStatus m_status;
#endif // GAME
	glm::vec3 m_position;
	AABB m_AABB;
	std::reference_wrapper<const Model> m_model;
};