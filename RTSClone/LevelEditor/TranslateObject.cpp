#include "TranslateObject.h"
#include "ModelManager.h"
#include "EntityManager.h"

TranslateObject::TranslateObject()
	: m_currentAxisSelected(eAxisCollision::None),
	m_selected(false),
	m_position(),
	m_xAABB(),
	m_zAABB()
{
	m_xAABB.reset({ m_position.x - 25.0 / 2.0f, m_position.y, m_position.z }, { 25.0f, 5.0f, 5.0f });
	m_zAABB.reset({ m_position.x, m_position.y, m_position.z + 25.0 / 2.0f }, { 5.0f, 5.0f, 25.0f });
}

eAxisCollision TranslateObject::getCurrentAxisSelected() const
{
	return m_currentAxisSelected;
}

bool TranslateObject::isSelected() const
{
	return m_selected;
}

bool TranslateObject::isSelected(const glm::vec3& position) const
{
	return m_xAABB.contains(position) || m_zAABB.contains(position);
}

const glm::vec3& TranslateObject::getPosition() const
{
	return m_position;
}

void TranslateObject::setSelected(bool selected, const glm::vec3& position)
{
	if (m_xAABB.contains(position))
	{
		m_selected = selected;
		m_currentAxisSelected = eAxisCollision::X;
	}
	else if (m_zAABB.contains(position))
	{
		m_selected = selected;
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
	m_xAABB.update(m_position);
	m_zAABB.update(m_position);
}

void TranslateObject::render(ShaderHandler& shaderHandler, const EntityManager& entityManager) const
{
	if (entityManager.isEntitySelected())
	{
		ModelManager::getInstance().getModel(eModelName::TranslateObject).render(shaderHandler, m_position);
	}
}

#ifdef RENDER_AABB
void TranslateObject::renderAABB(ShaderHandler& shaderHandler, const EntityManager& entityManager)
{
	if (entityManager.isEntitySelected())
	{
		m_xAABB.render(shaderHandler);
		m_zAABB.render(shaderHandler);
	}
}
#endif // RENDER_AABB