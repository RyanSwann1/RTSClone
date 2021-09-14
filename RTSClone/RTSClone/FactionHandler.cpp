#include "FactionHandler.h"
#include "FactionPlayer.h"
#include "Unit.h"

namespace
{
	int getFactionCount(const FactionsContainer& factions)
	{
		return static_cast<int>(std::count_if(factions.cbegin(), factions.cend(), [&](const auto& faction)
		{
			return faction.get();
		}));
	}
}

FactionHandler::FactionHandler(FactionsContainer&& factions)
	: m_factions(std::move(factions)),
	m_opposingFactions()
{
	m_opposingFactions.reserve(static_cast<size_t>(getFactionCount(m_factions)));
}

bool FactionHandler::isFactionActive(eFactionController factionController) const
{
	return m_factions[static_cast<int>(factionController)].get();
}

const FactionsContainer& FactionHandler::getFactions() const
{
	return m_factions;
}

FactionsContainer& FactionHandler::getFactions()
{
	return m_factions;
}

const std::vector<std::reference_wrapper<const Faction>>& FactionHandler::getOpposingFactions(eFactionController factionController)
{
	m_opposingFactions.clear();

	for (const auto& faction : m_factions)
	{
		if (faction && faction->getController() != factionController)
		{
			m_opposingFactions.push_back(*faction.get());
		}  
	}

	return m_opposingFactions;
}

const FactionPlayer* FactionHandler::getFactionPlayer() const
{
	Faction* factionPlayer = m_factions[static_cast<int>(eFactionController::Player)].get();
	return factionPlayer ? static_cast<const FactionPlayer*>(factionPlayer) : nullptr;
}

FactionPlayer* FactionHandler::getFactionPlayer()
{
	Faction* factionPlayer = m_factions[static_cast<int>(eFactionController::Player)].get();
	return factionPlayer ? static_cast<FactionPlayer*>(factionPlayer) : nullptr;
}

Faction* FactionHandler::getFaction(eFactionController factionController)
{
	return m_factions[static_cast<int>(factionController)].get();
}

const Faction* FactionHandler::getFaction(eFactionController factionController) const
{
	return m_factions[static_cast<int>(factionController)].get();
}

const Faction* FactionHandler::getRandomOpposingFaction(eFactionController senderFaction) const
{
	bool opposingFactionActive = std::any_of(m_factions.cbegin(), m_factions.cend(), [this, senderFaction](const auto& faction)
	{
		return faction.get() && faction.get()->getController() != senderFaction;
	});

	const Faction* opposingFaction = nullptr;
	if (getFactionCount(m_factions) > 1 && opposingFactionActive)
	{
		while (!opposingFaction)
		{
			int i = Globals::getRandomNumber(0, static_cast<int>(m_factions.size()) - 1);
			if (m_factions[i] && m_factions[i].get()->getController() != senderFaction)
			{
				opposingFaction = m_factions[i].get();
			}
		}
	}

	return opposingFaction;
}

bool FactionHandler::removeFaction(eFactionController faction)
{
	if (m_factions[static_cast<int>(faction)])
	{
		m_factions[static_cast<int>(faction)].reset();
		return true;
	}

	return false;
}