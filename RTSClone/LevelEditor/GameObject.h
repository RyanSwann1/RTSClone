#pragma once

#include "NonCopyable.h"
#include "AABB.h"
#include <functional>

class ShaderHandler;
struct Model;
class GameObject : private NonCopyable
{
public:
	GameObject(const Model& model);
	GameObject(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
	GameObject(GameObject&&) noexcept;
	GameObject& operator=(GameObject&&) noexcept;

	glm::vec3& getRotation();
	glm::vec3& getPosition();
	const Model& getModel() const;
	int getID() const;
	const glm::vec3& getRotation() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::vec3 rotation);

	void render(ShaderHandler& shaderHandler) const;
	void resetAABB();

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	AABB m_AABB;
	int m_ID;
	std::reference_wrapper<const Model> m_model;
};