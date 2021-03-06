#pragma once

#include "FactionController.h"
#include "EntityType.h"

class RepairTargetEntity
{
public:
	RepairTargetEntity();
	RepairTargetEntity(eEntityType entityType, int targetID);

	eEntityType getType() const;
	int getID() const;

	void set(eEntityType type, int ID);
	void reset();

private:
	eEntityType m_type;
	int m_ID;
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