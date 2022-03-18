#pragma once

#include "Faction.h"
#include <vector>
#include <memory>
#include <functional>

using FactionsContainer = std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>;
using opposing_factions = std::array<const Faction*, static_cast<size_t>(eFactionController::Max) + 1>;

struct LevelDetailsFromFile;
class BaseHandler;
class FactionPlayer;
class FactionHandler
{
public:
	FactionHandler(const BaseHandler& baseHandler, const LevelDetailsFromFile& levelDetails);
	FactionHandler(const FactionHandler&) = delete;
	FactionHandler& operator=(const FactionHandler&) = delete;
	FactionHandler(FactionHandler&&) noexcept = default;
	FactionHandler& operator=(FactionHandler&&) noexcept = default;

	bool isFactionActive(eFactionController factionController) const;
	
	const FactionsContainer& getFactions() const;
	FactionsContainer& getFactions();
	opposing_factions getOpposingFactions(eFactionController controller) const;
	const FactionPlayer* getFactionPlayer() const;
	FactionPlayer* getFactionPlayer();
	Faction* getFaction(eFactionController factionController);
	const Faction* getFaction(eFactionController factionController) const;
	const Faction* getRandomOpposingFaction(eFactionController senderFaction) const;

	bool removeFaction(eFactionController faction);

private:
	FactionsContainer m_factions = {};
};