#pragma once

#include "AABB.h"
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
	Mineral(Mineral&&) noexcept;
	Mineral& operator=(Mineral&&) = delete;
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
	bool m_active;
#endif // GAME
	glm::vec3 m_position;
	AABB m_AABB;
	std::reference_wrapper<const Model> m_model;
};