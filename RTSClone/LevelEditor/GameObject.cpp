#include "GameObject.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"

GameObject::GameObject(const Model& model)
	: m_position(),
	m_rotation(),
	m_AABB(),
	m_model(model)
{}

GameObject::GameObject(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: m_position(startingPosition),
	m_rotation(startingRotation),
	m_AABB(),
	m_model(model)
{
	m_AABB.reset(m_position, m_model);
}

GameObject::GameObject(GameObject&& rhs) noexcept
	: m_position(rhs.m_position),
	m_rotation(rhs.m_rotation),
	m_AABB(std::move(rhs.m_AABB)),
	m_model(rhs.m_model)
{}

GameObject& GameObject::operator=(GameObject&& rhs) noexcept
{
	assert(this != &rhs);
	m_position = rhs.m_position;
	m_rotation = rhs.m_rotation;
	m_AABB = std::move(rhs.m_AABB);
	m_model = rhs.m_model;

	return *this;
}

glm::vec3& GameObject::getRotation()
{
	return m_rotation;
}

const Model& GameObject::getModel() const
{
	return m_model;
}

void GameObject::setPosition(const glm::vec3& position)
{
	m_position = position;
	m_AABB.update(position);
}

void GameObject::setRotation(const glm::vec3 rotation)
{
	m_rotation = rotation;
}

const glm::vec3& GameObject::getRotation() const
{
	return m_rotation;
}

const glm::vec3& GameObject::getPosition() const
{
	return m_position;
}

const AABB& GameObject::getAABB() const
{
	return m_AABB;
}

void GameObject::render(ShaderHandler& shaderHandler, bool highlight) const
{
	m_model.get().render(shaderHandler, m_position, m_rotation);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB