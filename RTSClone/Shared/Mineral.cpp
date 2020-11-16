#include "Mineral.h"
#ifdef GAME
#include "GameMessenger.h"
#include "GameMessages.h"
#endif // GAME
#include "ModelManager.h"

#ifdef GAME
Mineral::Mineral(const glm::vec3& startingPosition)
	: Entity(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME), 
		startingPosition, eEntityType::Mineral)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}
#endif // GAME

#ifdef LEVEL_EDITOR
Mineral::Mineral(const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: Entity(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME), startingPosition, startingRotation)
{}
#endif // LEVEL_EDITOR

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
#ifdef GAME
	if (getID() != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
#endif // GAME
}