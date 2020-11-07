#include "FactionHandler.h"
#include "FactionPlayer.h"

namespace
{
	int getFactionCount(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions)
	{
		int factionCount = 0;
		for (const auto& faction : factions)
		{
			if (faction)
			{
				++factionCount;
			}
		}

		return factionCount;
	}
}

FactionHandler::FactionHandler(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& factions)
	: m_factions(factions),
	m_opposingFactions()
{
	m_opposingFactions.reserve(static_cast<size_t>(getFactionCount(m_factions)));
}

bool FactionHandler::isFactionActive(eFactionController factionController) const
{
	return m_factions[static_cast<int>(factionController)].get();
}

const std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>& FactionHandler::getFactions() const
{
	return m_factions;
}

const std::vector<const Faction*>& FactionHandler::getOpposingFactions(eFactionController factionController)
{
	m_opposingFactions.clear();

	for (const auto& faction : m_factions)
	{
		if (faction && faction->getController() != factionController)
		{
			m_opposingFactions.push_back(faction.get());
		}
	}

	return m_opposingFactions;
}

const Faction& FactionHandler::getFaction(eFactionController factionController) const
{
	assert(m_factions[static_cast<int>(factionController)] &&
		m_factions[static_cast<int>(factionController)]->getController() == factionController);

	return *m_factions[static_cast<int>(factionController)].get();
}

const Faction& FactionHandler::getRandomOpposingFaction(eFactionController senderFaction) const
{
	assert(getFactionCount(m_factions) >= 2);

	const Faction* opposingFaction = nullptr;
	while (!opposingFaction)
	{
		int i = Globals::getRandomNumber(0, static_cast<int>(m_factions.size()) - 1);
		if (m_factions[i] && m_factions[i].get()->getController() != senderFaction)
		{
			opposingFaction = m_factions[i].get();
		}
	}

	return *opposingFaction;
}