#include "FactionHandler.h"
#include "FactionPlayer.h"
#include "Entities/Unit.h"
#include "Base.h"
#include "Level.h"

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

FactionHandler::FactionHandler(const BaseHandler& baseHandler, const LevelDetailsFromFile& levelDetails)
{
	static_assert(static_cast<int>(AIConstants::eBehaviour::Max) == 1, "Current assigning of AI behaviour relies on only two behaviours");
	int AIBehaviourIndex = 0;

	assert(levelDetails.factionCount < static_cast<int>(eFactionController::Max) + 1 &&
		levelDetails.factionCount <= static_cast<int>(baseHandler.getBases().size()));
	for (int i = 0; i < levelDetails.factionCount; ++i)
	{
		assert(!m_factions[i]);
		switch (eFactionController(i))
		{
		case eFactionController::Player:
			m_factions[i] = std::make_unique<FactionPlayer>(baseHandler.getBases()[i].position,
				levelDetails.factionStartingResources, levelDetails.factionStartingPopulation);
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			m_factions[i] = std::make_unique<FactionAI>(eFactionController(i), baseHandler.getBases()[i].position,
				levelDetails.factionStartingResources, levelDetails.factionStartingPopulation,
				static_cast<AIConstants::eBehaviour>(AIBehaviourIndex), baseHandler);
			AIBehaviourIndex ^= 1;
			break;
		default:
			assert(false);
		}
	}
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

opposing_factions FactionHandler::getOpposingFactions(eFactionController controller) const
{
	opposing_factions opposingFactions = {};
	assert(opposingFactions.size() == m_factions.size());
	std::transform(m_factions.cbegin(), m_factions.cend(), opposingFactions.begin(), [controller](const auto& faction) -> const Faction*
	{
		if (faction && faction->getController() != controller)
		{
			return faction.get();
		}
		return nullptr;
	});

	return opposingFactions;
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
		return faction.get() && faction->getController() != senderFaction;
	});

	const Faction* opposingFaction = nullptr;
	if (opposingFactionActive && getFactionCount(m_factions) > 1)
	{
		while (!opposingFaction)
		{
			int i = Globals::getRandomNumber(0, static_cast<int>(m_factions.size()) - 1);
			if (m_factions[i] && m_factions[i]->getController() != senderFaction)
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