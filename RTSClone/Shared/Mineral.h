#pragma once

#include "Entity.h"
//#ifdef LEVEL_EDITOR

//
//class Mineral : public Entity
//{
//public:
//	Mineral(const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
//	~Mineral();
//};
//#endif // LEVEL_EDITOR

#ifdef GAME
#include "NonCopyable.h"
#include "AABB.h"
#include <functional>
struct Model;
class ShaderHandler;
class Mineral : private NonCopyable
{
public:
	Mineral(const glm::vec3& startingPosition);
	Mineral(Mineral&&) noexcept;
	Mineral& operator=(Mineral&&) = delete;
	~Mineral();	

	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;

	void render(ShaderHandler& shaderHandler) const;

private:
	bool m_active;
	glm::vec3 m_position;
	AABB m_AABB;
	std::reference_wrapper<const Model> m_model;
};
#endif // GAME