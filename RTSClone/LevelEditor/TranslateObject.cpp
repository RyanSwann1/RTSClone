#include "TranslateObject.h"
#include "ModelManager.h"
#include "EntityManager.h"

namespace 
{
	constexpr glm::vec3 X_OFF_SET{ 10.0f, 0.0f, 0.0f };
	constexpr glm::vec3 Z_OFF_SET{ 0.0f, 0.0f, -10.0f };
}

TranslateObject::TranslateObject()
	: m_currentAxisSelected(eAxisCollision::None),
	m_selected(false),
	m_position(),
	m_xAABB(),
	m_zAABB()
{
	m_xAABB.reset(m_position + X_OFF_SET, { 20.0f, 5.0f, 5.0f });
	m_zAABB.reset(m_position + Z_OFF_SET, { 5.0f, 5.0f, 20.0f });
}

eAxisCollision TranslateObject::getCurrentAxisSelected() const
{
	return m_currentAxisSelected;
}

bool TranslateObject::isSelected() const
{
	return m_selected;
}

const glm::vec3& TranslateObject::getPosition() const
{
	return m_position;
}

void TranslateObject::setSelected(const glm::vec3& position)
{
	if (m_xAABB.contains(position))
	{
		m_selected = true;
		m_currentAxisSelected = eAxisCollision::X;
	}
	else if (m_zAABB.contains(position))
	{
		m_selected = true;
		m_currentAxisSelected = eAxisCollision::Z;
	}
	else
	{
		m_selected = false;
		m_currentAxisSelected = eAxisCollision::None;
	}
}

void TranslateObject::deselect()
{
	m_selected = false;
}

void TranslateObject::setPosition(const glm::vec3& position)
{
	m_position = position;
	m_xAABB.update(m_position + X_OFF_SET);
	m_zAABB.update(m_position + Z_OFF_SET);
}

void TranslateObject::render(ShaderHandler& shaderHandler, bool active) const
{
	if (active)
	{
		ModelManager::getInstance().getModel(eModelName::TranslateObject).render(shaderHandler, m_position);
	}
}

#ifdef RENDER_AABB
void TranslateObject::renderAABB(ShaderHandler& shaderHandler, bool active)
{
	if (active)
	{
		m_xAABB.render(shaderHandler);
		m_zAABB.render(shaderHandler);
	}
}
#endif // RENDER_AABB