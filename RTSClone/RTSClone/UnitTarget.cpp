#include "UnitTarget.h"
#include "Globals.h"

UnitTarget::UnitTarget()
	: m_factionController(),
	m_ID(Globals::INVALID_ENTITY_ID)
{}

UnitTarget::UnitTarget(eFactionController factionController, int targetID)
	: m_factionController(factionController),
	m_ID(targetID)
{}

eFactionController UnitTarget::getFactionController() const
{
	return m_factionController;
}

int UnitTarget::getID() const
{
	return m_ID;
}

void UnitTarget::set(eFactionController factionController, int ID)
{
	m_factionController = factionController;
	m_ID = ID;
}

void UnitTarget::reset()
{
	m_ID = Globals::INVALID_ENTITY_ID;
}
