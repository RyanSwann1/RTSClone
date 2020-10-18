#include "TranslateObject.h"
#include "ModelManager.h"
#include <array>

namespace
{
	const glm::vec3 X_STARTING_DIRECTION = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 Y_STARTING_DIRECTION = { 0.0f, 1.0f, 0.0f };
	const glm::vec3 Z_STARTING_DIRECTION = { 0.0f, 0.0f, 1.0f };
}

TranslateObject::TranslateObject()
	: m_centerPosition(),
	m_mesh(),
	m_xPosition(),
	m_xAABB(),
	m_yPosition(),
	m_yAABB(),
	m_zPosition(),
	m_zAABB()
{
	m_xAABB.reset(m_centerPosition, { 25.0f, 5.0f, 5.0f });
	m_yAABB.reset(m_centerPosition, { 5.0f, 25.0f, 5.0f });
	m_zAABB.reset(m_centerPosition, { 5.0f, 5.0f, -25.0f });
}

void TranslateObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(eModelName::TranslateObject).render(shaderHandler, m_centerPosition);
}

#ifdef RENDER_AABB
void TranslateObject::renderAABB(ShaderHandler& shaderHandler)
{
	m_xAABB.render(shaderHandler);
	m_yAABB.render(shaderHandler);
	m_zAABB.render(shaderHandler);
}
#endif // RENDER_AABB