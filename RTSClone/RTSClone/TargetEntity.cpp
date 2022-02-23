#include "TargetEntity.h"
#include "Globals.h"
#include "FactionHandler.h"

RepairTargetEntity::RepairTargetEntity(eEntityType type, int ID)
	: type(type),
	ID(ID)
{}

TargetEntity::TargetEntity(eFactionController controller, int ID, eEntityType type)
	: controller(controller),
	ID(ID),
	type(type)
{}