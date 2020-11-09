#include "SceneryGameObject.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Globals.h"

SceneryGameObject::SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation)
	: m_model(model),
	m_position(position),
	m_rotation(rotation),
	m_active(true)
{
	AABB AABB(m_position, m_model);
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ AABB });
}

SceneryGameObject::~SceneryGameObject()
{
	if (m_active)
	{
		AABB AABB(m_position, m_model);
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ AABB });
	}
}

SceneryGameObject::SceneryGameObject(SceneryGameObject&& orig) noexcept
	: m_model(orig.m_model),
	m_position(orig.m_position),
	m_rotation(orig.m_rotation),
	m_active(orig.m_active)
{
	orig.m_active = false;
}

SceneryGameObject& SceneryGameObject::operator=(SceneryGameObject&& orig) noexcept
{
	m_model = orig.m_model;
	m_position = orig.m_position;
	m_rotation = orig.m_rotation;
	m_active = orig.m_active;

	orig.m_active = false;

	return *this;
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