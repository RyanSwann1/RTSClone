#include "Mineral.h"
#include "GameEventMessenger.h"
#include "GameEvents.h"

Mineral::Mineral(const glm::vec3& startingPosition)
	: Entity(startingPosition, eModelName::Mineral, eEntityType::Mineral)
{
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::AddEntityToMap>>({ m_AABB });
}

Mineral::Mineral(Mineral&& orig) noexcept
	: Entity(std::move(orig))
{}

Mineral& Mineral::operator=(Mineral&& orig) noexcept
{
	Entity::operator=(std::move(orig));
	return *this;
}

Mineral::~Mineral()
{
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::RemoveEntityFromMap>>({ m_AABB });
}