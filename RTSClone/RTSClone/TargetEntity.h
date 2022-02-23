#pragma once

#include "FactionController.h"
#include "EntityType.h"

struct RepairTargetEntity
{
	RepairTargetEntity(eEntityType type, int ID);

	const eEntityType type;
	const int ID;
};

struct TargetEntity
{
	TargetEntity(eFactionController controller, int ID, eEntityType type);

	const eFactionController controller;
	const int ID;
	const eEntityType type;
};