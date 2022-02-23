#include "TargetEntity.h"
#include "Globals.h"
#include "FactionHandler.h"

//TargetEntity
TargetEntity::TargetEntity()
	: m_targetFactionController(),
	m_targetID(Globals::INVALID_ENTITY_ID),
	m_targetType()
{}

TargetEntity::TargetEntity(eFactionController targetFactionController, int targetID, eEntityType targetType)
	: m_targetFactionController(targetFactionController),
	m_targetID(targetID),
	m_targetType(targetType)
{}

eEntityType TargetEntity::getType() const
{
	return m_targetType;
}

eFactionController TargetEntity::getFactionController() const
{
	return m_targetFactionController;
}

int TargetEntity::getID() const
{
	return m_targetID;
}

void TargetEntity::set(eFactionController targetFactionController, int ID, eEntityType type)
{
	m_targetFactionController = targetFactionController;
	m_targetID = ID;
	m_targetType = type;
}

void TargetEntity::reset()
{
	m_targetID = Globals::INVALID_ENTITY_ID;
}

RepairTargetEntity::RepairTargetEntity(eEntityType type, int ID)
	: type(type),
	ID(ID)
{}