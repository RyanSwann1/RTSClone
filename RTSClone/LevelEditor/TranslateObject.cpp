#include "TranslateObject.h"
#include "ModelManager.h"

TranslateObject::TranslateObject()
	: m_currentAxisSelected(eAxisCollision::None),
	m_selected(false),
	m_active(false),
	m_centerPosition(),
	m_xPosition(),
	m_xAABB(),
	m_zPosition(),
	m_zAABB()
{
	m_xAABB.reset({ m_centerPosition.x - 25.0 / 2.0f, m_centerPosition.y, m_centerPosition.z }, { 25.0f, 5.0f, 5.0f });
	m_zAABB.reset({ m_centerPosition.x, m_centerPosition.y, m_centerPosition.z + 25.0 / 2.0f }, { 5.0f, 5.0f, 25.0f });
}

eAxisCollision TranslateObject::getAxisCollision() const
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

const glm::vec3& TranslateObject::getCenterPosition() const
{
	return m_centerPosition;
}

eAxisCollision TranslateObject::getCollisionType(const glm::vec3& mouseToGroundPosition) const
{
	if (m_xAABB.contains(mouseToGroundPosition))
	{
		return eAxisCollision::X;
	}
	else if (m_zAABB.contains(mouseToGroundPosition))
	{
		return eAxisCollision::Z;
	}

	return eAxisCollision::None;
}

void TranslateObject::setSelected(bool selected, const glm::vec3& position)
{
	m_selected = selected;
	if (m_xAABB.contains(position))
	{
		m_currentAxisSelected = eAxisCollision::X;
	}
	else if (m_zAABB.contains(position))
	{
		m_currentAxisSelected = eAxisCollision::Z;
	}
}

void TranslateObject::setSelected(bool selected)
{
	m_selected = false;
}

void TranslateObject::setActive(bool active)
{
	m_active = active;
}

void TranslateObject::setPosition(const glm::vec3& position)
{
	m_centerPosition = position;
	m_xAABB.update(m_centerPosition);
	m_zAABB.update(m_centerPosition);
}

void TranslateObject::setPosition(eAxisCollision axisCollision, const glm::vec3& position)
{
	switch (axisCollision)
	{
	case eAxisCollision::X:
		m_centerPosition.x = position.x;
		break;
	case eAxisCollision::Z:
		m_centerPosition.z = position.z;
		break;
	}

	m_xAABB.update(m_centerPosition);
	m_zAABB.update(m_centerPosition);
}

void TranslateObject::render(ShaderHandler& shaderHandler) const
{
	//if (m_active)
	//{
		ModelManager::getInstance().getModel(eModelName::TranslateObject).render(shaderHandler, m_centerPosition);
	//}
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