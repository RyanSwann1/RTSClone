#include "Mineral.h"
#include "ModelManager.h"
#ifdef GAME
#include "GameMessenger.h"
#include "GameMessages.h"
#endif // GAME

#ifdef LEVEL_EDITOR
Mineral::Mineral()
	: m_position(0.0f),
	m_AABB(),
	m_model(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME))
{
	m_AABB.reset(m_position, m_model);
}

Mineral::Mineral(const glm::vec3& startingPosition)
	: m_position(startingPosition),
	m_AABB(),
	m_model(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME))
{
	m_AABB.reset(m_position, m_model);
}
#endif // LEVEL_EDITOR

#ifdef GAME
Mineral::Mineral(const glm::vec3& startingPosition, int quantity)
	: m_quantity(quantity),
	m_position(Globals::convertToMiddleGridPosition(startingPosition)),
	m_AABB(),
	m_model(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME))
{
	m_AABB.reset(m_position, m_model);
}

int Mineral::getQuantity() const
{
	return m_quantity;
}

int Mineral::extractQuantity(int quantityToExtract) const
{
	m_quantity = std::max(0, m_quantity - quantityToExtract);
	return m_quantity > 0 ? quantityToExtract : m_quantity;
}
#endif // GAME

const glm::vec3& Mineral::getPosition() const
{
	return m_position;
}

const AABB& Mineral::getAABB() const
{
	return m_AABB;
}

#ifdef LEVEL_EDITOR
void Mineral::setPosition(const glm::vec3& position)
{
	m_position = position;
	m_AABB.update(m_position);
}
#endif // LEVEL_EDITOR

void Mineral::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, m_position);
}