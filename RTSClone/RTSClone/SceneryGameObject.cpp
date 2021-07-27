#include "SceneryGameObject.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Globals.h"
#include "Model.h"

SceneryGameObject::SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation,
	const glm::vec3& scale, float left, float right, float forward, float back)
	: m_model(model),
	m_AABB(left, right, forward, back),
	m_position(position),
	m_rotation(rotation),
	m_scale(scale),
	m_active()
{
	broadcastToMessenger<GameMessages::AddAABBToMap>({ m_AABB });
}

SceneryGameObject::~SceneryGameObject()
{
	if (m_active.isActive())
	{
		broadcastToMessenger<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

const glm::vec3& SceneryGameObject::getScale() const
{
	return m_scale;
}

const glm::vec3& SceneryGameObject::getRotation() const
{
	return m_rotation;
}

const glm::vec3& SceneryGameObject::getPosition() const
{
	return m_position;
}

const AABB & SceneryGameObject::getAABB() const
{
	return m_AABB;
}

void SceneryGameObject::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void SceneryGameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB AABB(m_position, m_model);
	AABB.render(shaderHandler);
}
#endif // RENDER_AABB