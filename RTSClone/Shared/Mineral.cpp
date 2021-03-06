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
Mineral::Mineral(const glm::vec3& startingPosition)
	: m_status(),// m_active(true),
	m_position(Globals::convertToMiddleGridPosition(startingPosition)),
	m_AABB(),
	m_model(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME))
{
	m_AABB.reset(m_position, m_model);
	broadcastToMessenger<GameMessages::AddAABBToMap>({ m_AABB });
}

Mineral::~Mineral()
{
	if (m_status.isActive())
	{
		broadcastToMessenger<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
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