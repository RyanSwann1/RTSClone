#include "Mineral.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"

Mineral::Mineral(const glm::vec3& startingPosition)
	: Entity(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(), startingPosition, eModelName::Mineral, eEntityType::Mineral)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
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
	if (getID() != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
}