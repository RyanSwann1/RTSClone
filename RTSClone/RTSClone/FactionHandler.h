#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Faction.h"
#include <vector>
#include <memory>
#include <functional>

class Unit;
class FactionHandler : private NonCopyable, private NonMovable
{
public:
	FactionHandler(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions);

	bool isFactionActive(eFactionController factionController) const;
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& getFactions() const;
	const std::vector<std::reference_wrapper<const Faction>>& getOpposingFactions(eFactionController factionController);

	const Faction& getFaction(eFactionController factionController) const;
	const Faction& getRandomOpposingFaction(eFactionController senderFaction) const;

private:
	const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& m_factions;
	std::vector<std::reference_wrapper<const Faction>> m_opposingFactions;
};