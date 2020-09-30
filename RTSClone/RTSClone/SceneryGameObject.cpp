#include "SceneryGameObject.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"
#include "Globals.h"

SceneryGameObject::SceneryGameObject(eModelName modelName, const glm::vec3& position)
	: m_modelName(modelName),
	m_position(position),
	m_active(true)
{
	AABB AABB(m_position, ModelManager::getInstance().getModel(m_modelName));
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ AABB });	
}

SceneryGameObject::~SceneryGameObject()
{
	if (m_active)
	{
		AABB AABB(m_position, ModelManager::getInstance().getModel(m_modelName));
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ AABB });
	}
}

SceneryGameObject::SceneryGameObject(SceneryGameObject&& orig) noexcept
	: m_modelName(orig.m_modelName),
	m_position(orig.m_position),
	m_active(orig.m_active)
{
	orig.m_active = false;
}

SceneryGameObject& SceneryGameObject::operator=(SceneryGameObject&& orig) noexcept
{
	m_modelName = orig.m_modelName;
	m_position = orig.m_position;
	m_active = orig.m_active;

	orig.m_active = false;

	return *this;
}

void SceneryGameObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(m_modelName).render(shaderHandler, m_position);
}

#ifdef RENDER_AABB
void SceneryGameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB AABB(m_position, ModelManager::getInstance().getModel(m_modelName));
	AABB.render(shaderHandler);
}
#endif // RENDER_AABB