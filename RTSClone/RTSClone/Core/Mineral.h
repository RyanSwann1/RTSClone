#pragma once

#include "Core/AABB.h"
#include <functional>

struct Model;
class ShaderHandler;
class Mineral
{
public:
#ifdef LEVEL_EDITOR
	Mineral();
	Mineral(const glm::vec3& startingPosition);
#endif // LEVEL_EDITOR
	Mineral(const glm::vec3& startingPosition, int quantity);
	int getQuantity() const;
	int extractQuantity(int quantityToExtract) const;

	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
#endif // LEVEL_EDITOR
	void render(ShaderHandler& shaderHandler) const;

private:
#ifdef GAME
	mutable int m_quantity;
#endif // GAME
	glm::vec3 m_position;
	AABB m_AABB;
	std::reference_wrapper<const Model> m_model;
};