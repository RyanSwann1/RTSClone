#pragma once

#include "FactionController.h"

struct UnitTarget
{
	UnitTarget();
	UnitTarget(eFactionController factionController, int targetID);

	eFactionController factionController;
	int ID;
};