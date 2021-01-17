#include "Mineral.h"
#include "ModelManager.h"

#ifdef GAME
#include "GameMessenger.h"
#include "GameMessages.h"

Mineral::Mineral(const glm::vec3& startingPosition)
	: m_active(true),
	m_position(startingPosition),
	m_AABB(),
	m_model(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME))
{
	m_position = Globals::convertToMiddleGridPosition(startingPosition);
	m_AABB.reset(m_position, m_model);
	broadcastToMessenger<GameMessages::AddToMap>({ m_AABB });
}

Mineral::Mineral(Mineral&& rhs) noexcept
	: m_active(rhs.m_active),
	m_position(rhs.m_position),
	m_AABB(rhs.m_AABB),
	m_model(rhs.m_model)
{
	rhs.m_active = false;
}

Mineral::~Mineral()
{
	if (m_active)
	{
		broadcastToMessenger<GameMessages::RemoveFromMap>({ m_AABB });
	}
}

const glm::vec3& Mineral::getPosition() const
{
	return m_position;
}

const AABB& Mineral::getAABB() const
{
	return m_AABB;
}

void Mineral::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, m_position);
}
#endif // GAME