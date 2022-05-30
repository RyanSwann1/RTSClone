#pragma once

#include "Faction.h"
#include <vector>
#include <memory>

struct LevelDetailsFromFile;
class BaseHandler;
class FactionPlayer;
class FactionHandler
{
public:
	FactionHandler(const BaseHandler& baseHandler, const LevelDetailsFromFile& levelDetails);
	FactionHandler(FactionHandler&) = delete;
	FactionHandler& operator=(FactionHandler&) = delete;
	FactionHandler(FactionHandler&&) noexcept = default;
	FactionHandler& operator=(FactionHandler&&) noexcept = default;

	bool isFactionActive(eFactionController factionController) const;

	const std::vector<std::unique_ptr<Faction>>& getFactions() const;
	std::vector<std::unique_ptr<Faction>>& getFactions();
	const std::vector<const Faction*>& GetOpposingFactions(eFactionController controller);
	const FactionPlayer* getFactionPlayer() const;
	FactionPlayer* getFactionPlayer();
	Faction* getFaction(eFactionController factionController);
	const Faction* getFaction(eFactionController factionController) const;
	const Faction* getRandomOpposingFaction(eFactionController senderFaction) const;

	bool removeFaction(eFactionController faction);

private:
	std::vector<std::unique_ptr<Faction>> m_factions{};
	std::vector<const Faction*> m_opposing_factions{};
};