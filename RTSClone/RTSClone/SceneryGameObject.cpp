#include "SceneryGameObject.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Globals.h"
#include "Model.h"

SceneryGameObject::SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation)
	: m_model(model),
	m_AABB(position, model),
	m_position(position),
	m_rotation(rotation),
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
	m_model.get().render(shaderHandler, m_position, m_rotation);
}

#ifdef RENDER_AABB
void SceneryGameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB AABB(m_position, m_model);
	AABB.render(shaderHandler);
}
#endif // RENDER_AABB