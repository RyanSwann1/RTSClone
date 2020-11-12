#include "TargetEntity.h"
#include "Globals.h"
#include "FactionHandler.h"

TargetEntity::TargetEntity()
	: m_targetFactionController(),
	m_targetID(Globals::INVALID_ENTITY_ID)
{}

TargetEntity::TargetEntity(eFactionController targetFactionController, int targetID)
	: m_targetFactionController(targetFactionController),
	m_targetID(targetID)
{}

eFactionController TargetEntity::getFactionController() const
{
	return m_targetFactionController;
}

int TargetEntity::getID() const
{
	return m_targetID;
}

void TargetEntity::set(eFactionController targetFactionController, int ID)
{
	m_targetFactionController = targetFactionController;
	m_targetID = ID;
}

void TargetEntity::reset()
{
	m_targetID = Globals::INVALID_ENTITY_ID;
}