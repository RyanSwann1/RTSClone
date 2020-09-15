#include "FactionHandler.h"
#include "FactionPlayer.h"

FactionHandler::FactionHandler(const std::vector<std::unique_ptr<Faction>>& factions)
	: m_factions(factions),
	m_opposingFactions()
{
	m_opposingFactions.reserve(m_factions.size() - static_cast<size_t>(1));
}

const std::vector<std::unique_ptr<Faction>>& FactionHandler::getFactions() const
{
	return m_factions;
}

const std::vector<const Faction*>& FactionHandler::getOpposingFactions(eFactionController factionController)
{
	m_opposingFactions.clear();

	for (const auto& faction : m_factions)
	{
		if (faction->getController() != factionController)
		{
			m_opposingFactions.push_back(faction.get());
		}
	}

	return m_opposingFactions;
}

const Faction& FactionHandler::getFaction(eFactionController factionController) const
{
	assert(static_cast<int>(factionController) < static_cast<int>(m_factions.size()) &&
		m_factions[static_cast<int>(factionController)]->getController() == factionController);

	return *m_factions[static_cast<int>(factionController)].get();
}

Faction& FactionHandler::fgetFaction(eFactionController factionController)
{
	assert(static_cast<int>(factionController) < static_cast<int>(m_factions.size()) &&
		m_factions[static_cast<int>(factionController)]->getController() == factionController);

	return *m_factions[static_cast<int>(factionController)].get();
}