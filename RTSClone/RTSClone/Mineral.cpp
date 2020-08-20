#include "Mineral.h"
#include "GameMessenger.h"
#include "GameEvents.h"
#include "Globals.h"

Mineral::Mineral(int ID, const glm::vec3& startingPosition)
	: Entity(ID, startingPosition, eModelName::Mineral, eEntityType::Mineral)
{
	GameMessenger::getInstance().broadcast<GameEvents::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
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
	if (m_ID != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
}