#include "TargetEntity.h"
#include "Globals.h"

TargetEntity::TargetEntity()
	: m_factionController(),
	m_ID(Globals::INVALID_ENTITY_ID)
{}

TargetEntity::TargetEntity(eFactionController factionController, int targetID)
	: m_factionController(factionController),
	m_ID(targetID)
{}

eFactionController TargetEntity::getFactionController() const
{
	return m_factionController;
}

int TargetEntity::getID() const
{
	return m_ID;
}

void TargetEntity::set(eFactionController factionController, int ID)
{
	m_factionController = factionController;
	m_ID = ID;
}

void TargetEntity::reset()
{
	m_ID = Globals::INVALID_ENTITY_ID;
}
