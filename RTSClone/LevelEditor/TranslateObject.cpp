#include "TranslateObject.h"
#include "ModelManager.h"

TranslateObject::TranslateObject()
	: m_active(false),
	m_centerPosition(),
	m_xPosition(),
	m_xAABB(),
	m_zPosition(),
	m_zAABB()
{
	m_xAABB.reset({ m_centerPosition.x - 25.0 / 2.0f, m_centerPosition.y, m_centerPosition.z }, { 25.0f, 5.0f, 5.0f });
	m_zAABB.reset({ m_centerPosition.x, m_centerPosition.y, m_centerPosition.z + 25.0 / 2.0f }, { 5.0f, 5.0f, 25.0f });
}

bool TranslateObject::isColliding(const glm::vec3& mouseToGroundPosition) const
{
	return m_xAABB.contains(mouseToGroundPosition) || m_zAABB.contains(mouseToGroundPosition);
}

void TranslateObject::setActive(bool active)
{
	m_active = active;
}

void TranslateObject::setPosition(const glm::vec3& position)
{
	m_centerPosition = position;
	m_xAABB.update(position);
	m_zAABB.update(position);
}

void TranslateObject::render(ShaderHandler& shaderHandler) const
{
	if (m_active)
	{
		ModelManager::getInstance().getModel(eModelName::TranslateObject).render(shaderHandler, m_centerPosition);
	}
}

#ifdef RENDER_AABB
void TranslateObject::renderAABB(ShaderHandler& shaderHandler)
{
	if (m_active)
	{
		m_xAABB.render(shaderHandler);
		m_zAABB.render(shaderHandler);
	}
}
#endif // RENDER_AABB