#pragma once

#include "Faction.h"
#include <vector>
#include <memory>
#include <functional>

class Unit;
class FactionHandler
{
public:
	FactionHandler(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);
	FactionHandler(const FactionHandler&) = delete;
	FactionHandler& operator=(const FactionHandler&) = delete;
	FactionHandler(FactionHandler&&) = delete;
	FactionHandler& operator=(FactionHandler&&) = delete;

	bool isFactionActive(eFactionController factionController) const;
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& getFactions() const;
	const std::vector<std::reference_wrapper<const Faction>>& getOpposingFactions(eFactionController factionController);

	const Faction& getFaction(eFactionController factionController) const;
	const Faction& getRandomOpposingFaction(eFactionController senderFaction) const;

private:
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& m_factions;
	std::vector<std::reference_wrapper<const Faction>> m_opposingFactions;
};