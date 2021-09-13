#pragma once

#include "Faction.h"
#include <vector>
#include <memory>
#include <functional>

using FactionsContainer = std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>;

class FactionPlayer;
class Unit;
class FactionHandler
{
public:
	FactionHandler(FactionsContainer&& factions);
	FactionHandler(const FactionHandler&) = delete;
	FactionHandler& operator=(const FactionHandler&) = delete;
	FactionHandler(FactionHandler&&) = delete;
	FactionHandler& operator=(FactionHandler&&) = delete;

	bool isFactionActive(eFactionController factionController) const;
	
	const FactionsContainer& getFactions() const;
	FactionsContainer& getFactions();
	const std::vector<std::reference_wrapper<const Faction>>& getOpposingFactions(eFactionController factionController);
	const FactionPlayer* getFactionPlayer() const;
	FactionPlayer* getFactionPlayer();
	Faction* getFaction(eFactionController factionController);
	const Faction* getFaction(eFactionController factionController) const;
	const Faction* getRandomOpposingFaction(eFactionController senderFaction) const;

	bool removeFaction(eFactionController faction);

private:
	FactionsContainer m_factions;
	std::vector<std::reference_wrapper<const Faction>> m_opposingFactions;
};