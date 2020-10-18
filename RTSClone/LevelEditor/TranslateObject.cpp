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
{}

void TranslateObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(eModelName::Translate).render(shaderHandler, m_centerPosition);
}