#pragma once

#include "FactionController.h"

class FactionHandler;
class TargetEntity
{
public:
	TargetEntity();
	TargetEntity(eFactionController targetFactionController, int targetID);

	eFactionController getFactionController() const;
	int getID() const;

	void set(eFactionController targetFactionController, int ID);
	void reset();

private:
	eFactionController m_targetFactionController;
	int m_targetID;
};