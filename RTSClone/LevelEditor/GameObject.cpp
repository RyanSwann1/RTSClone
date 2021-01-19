#include "GameObject.h"
#include "UniqueIDGenerator.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"

GameObject::GameObject(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: m_position(startingPosition),
	m_rotation(startingRotation),
	m_AABB(),
	m_ID(UniqueIDGenerator::getInstance().getUniqueID()),
	m_model(model),
	m_selected(false)
{
	m_AABB.reset(m_position, m_model);
}

GameObject::GameObject(GameObject&& rhs) noexcept
	: m_position(rhs.m_position),
	m_rotation(rhs.m_rotation),
	m_AABB(std::move(rhs.m_AABB)),
	m_ID(rhs.m_ID),
	m_model(rhs.m_model),
	m_selected(rhs.m_selected)
{
	rhs.m_ID = Globals::INVALID_GAMEOBJECT_ID;
}

GameObject& GameObject::operator=(GameObject&& rhs) noexcept
{
	assert(this != &rhs);
	m_position = rhs.m_position;
	m_rotation = rhs.m_rotation;
	m_AABB = std::move(rhs.m_AABB);
	m_ID = rhs.m_ID;
	m_model = rhs.m_model;
	m_selected = rhs.m_selected;

	rhs.m_ID = Globals::INVALID_GAMEOBJECT_ID;

	return *this;
}

glm::vec3& GameObject::getRotation()
{
	return m_rotation;
}

glm::vec3& GameObject::getPosition()
{
	return m_position;
}

const Model& GameObject::getModel() const
{
	return m_model;
}

void GameObject::setPosition(const glm::vec3& position)
{
	m_position = position;
}

void GameObject::setRotation(const glm::vec3 rotation)
{
	m_rotation = rotation;
}

void GameObject::resetAABB()
{
	m_AABB.reset();
}

int GameObject::getID() const
{
	return m_ID;
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

bool GameObject::isSelected() const
{
	return m_selected;
}

void GameObject::setSelected(bool selected)
{
	m_selected = selected;
}

void GameObject::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, m_position, m_rotation);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB