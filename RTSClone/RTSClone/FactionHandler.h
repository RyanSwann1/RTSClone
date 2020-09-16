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
	FactionHandler(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);

	bool isFactionActive(eFactionController factionController) const;
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& getFactions() const;
	const std::vector<const Faction*>& getOpposingFactions(eFactionController factionController);
	const Faction& getFaction(eFactionController factionController) const;

private:
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& m_factions;
	std::vector<const Faction*> m_opposingFactions;

	Faction& fgetFaction(eFactionController factionController);
};