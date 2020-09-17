#include "EntityTarget.h"
#include "Globals.h"

EntityTarget::EntityTarget()
	: m_factionController(),
	m_ID(Globals::INVALID_ENTITY_ID)
{}

EntityTarget::EntityTarget(eFactionController factionController, int targetID)
	: m_factionController(factionController),
	m_ID(targetID)
{}

eFactionController EntityTarget::getFactionController() const
{
	return m_factionController;
}

int EntityTarget::getID() const
{
	return m_ID;
}

void EntityTarget::set(eFactionController factionController, int ID)
{
	m_factionController = factionController;
	m_ID = ID;
}

void EntityTarget::reset()
{
	m_ID = Globals::INVALID_ENTITY_ID;
}
