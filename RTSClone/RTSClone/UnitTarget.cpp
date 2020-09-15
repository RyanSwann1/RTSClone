#include "UnitTarget.h"
#include "Globals.h"

UnitTarget::UnitTarget()
	: factionController(),
	ID(Globals::INVALID_ENTITY_ID)
{}

UnitTarget::UnitTarget(eFactionController factionController, int targetID)
	: factionController(factionController),
	ID(targetID)
{}