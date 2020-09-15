#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Faction.h"
#include <vector>
#include <memory>

class FactionHandler : private NonCopyable, private NonMovable
{
	friend class GameEventHandler;
public:
	FactionHandler(const std::vector<std::unique_ptr<Faction>>& factions);

	const std::vector<std::unique_ptr<Faction>>& getFactions() const;
	const std::vector<const Faction*>& getOpposingFactions(eFactionController factionController);
	const Faction& getFaction(eFactionController factionController) const;

private:
	const std::vector<std::unique_ptr<Faction>>& m_factions;
	std::vector<const Faction*> m_opposingFactions;

	Faction& fgetFaction(eFactionController factionController);
};