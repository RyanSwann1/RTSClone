#pragma once

#include "FactionController.h"
#include "EntityType.h"

struct RepairTargetEntity
{
	RepairTargetEntity(eEntityType type, int ID);

	eEntityType type;
	int ID;
};

struct TargetEntity
{
	TargetEntity(eFactionController controller, int ID, eEntityType type);

	eFactionController controller;
	int ID;
	eEntityType type;
};