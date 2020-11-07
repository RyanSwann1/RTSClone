#pragma once

#include "FactionController.h"

class TargetEntity
{
public:
	TargetEntity();
	TargetEntity(eFactionController factionController, int targetID);

	eFactionController getFactionController() const;
	int getID() const;

	void set(eFactionController factionController, int ID);
	void reset();

private:
	eFactionController m_factionController;
	int m_ID;
};