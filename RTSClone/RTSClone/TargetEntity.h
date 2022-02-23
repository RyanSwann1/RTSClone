#pragma once

#include "FactionController.h"
#include "EntityType.h"

struct RepairTargetEntity
{
	RepairTargetEntity(eEntityType type, int ID);

	const eEntityType type;
	const int ID;
};

class TargetEntity
{
public:
	TargetEntity();
	TargetEntity(eFactionController targetFactionController, int targetID, eEntityType targetType);

	eEntityType getType() const;
	eFactionController getFactionController() const;
	int getID() const;

	void set(eFactionController targetFactionController, int ID, eEntityType type);
	void reset();

private:
	eFactionController m_targetFactionController;
	int m_targetID;
	eEntityType m_targetType;
};