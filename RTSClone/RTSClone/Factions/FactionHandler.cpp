#include "FactionHandler.h"
#include "FactionPlayer.h"
#include "Entities/Unit.h"
#include "Core/Base.h"
#include "Core/Level.h"

namespace
{
	std::vector<std::unique_ptr<Faction>>::iterator GetFaction(std::vector<std::unique_ptr<Faction>>& factions, 
		const eFactionController controller)
	{
		const auto faction = std::find_if(factions.begin(), factions.end(), [controller](const auto& faction)
		{
			return faction->getController() == controller;
		});
		return faction;
	}

	std::vector<std::unique_ptr<Faction>>::const_iterator GetFaction(const std::vector<std::unique_ptr<Faction>>& factions,
		const eFactionController controller)
	{
		const auto faction = std::find_if(factions.cbegin(), factions.cend(), [controller](const auto& faction)
		{
			return faction->getController() == controller;
		});
		return faction;
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
		switch (eFactionController(i))
		{
		case eFactionController::Player:
			m_factions.emplace_back(std::make_unique<FactionPlayer>(baseHandler.getBases()[i].position,
				levelDetails.factionStartingResources, levelDetails.factionStartingPopulation));
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			m_factions.emplace_back(std::make_unique<FactionAI>(eFactionController(i), baseHandler.getBases()[i].position,
				levelDetails.factionStartingResources, levelDetails.factionStartingPopulation,
				static_cast<AIConstants::eBehaviour>(AIBehaviourIndex), baseHandler));
			AIBehaviourIndex ^= 1;
			break;
		default:
			assert(false);
		}
	}

	m_opposing_factions.reserve(m_factions.size());
}

bool FactionHandler::isFactionActive(eFactionController factionController) const
{
	const auto faction = GetFaction(m_factions, factionController);
	return faction != m_factions.cend();
}

const std::vector<std::unique_ptr<Faction>>& FactionHandler::getFactions() const
{
	return m_factions;
}

std::vector<std::unique_ptr<Faction>>& FactionHandler::getFactions()
{
	return m_factions;
}

const std::vector<const Faction*>& FactionHandler::GetOpposingFactions(eFactionController controller)
{
	m_opposing_factions.clear();
	for (const auto& faction : m_factions)
	{
		if (faction->getController() != controller)
		{
			m_opposing_factions.push_back(faction.get());
		}
	}

	return m_opposing_factions;
}

const FactionPlayer* FactionHandler::getFactionPlayer() const
{
	const auto faction = GetFaction(m_factions, eFactionController::Player);
	if (faction != m_factions.cend())
	{
		return static_cast<const FactionPlayer*>((*faction).get());
	}

	return nullptr;
}

FactionPlayer* FactionHandler::getFactionPlayer()
{
	const auto faction = GetFaction(m_factions, eFactionController::Player);
	if (faction != m_factions.cend())
	{
		return static_cast<FactionPlayer*>((*faction).get());
	}

	return nullptr;
}

Faction* FactionHandler::getFaction(eFactionController factionController)
{
	const auto faction = GetFaction(m_factions, factionController);
	if (faction != m_factions.end())
	{
		return (*faction).get();
	}

	return nullptr;
}

const Faction* FactionHandler::getFaction(eFactionController factionController) const
{
	const auto faction = GetFaction(m_factions, factionController);
	if (faction != m_factions.cend())
	{
		return (*faction).get();
	}

	return nullptr;
}

const Faction* FactionHandler::getRandomOpposingFaction(eFactionController senderFaction) const
{
	assert(m_factions.size() > 1);
	const Faction* opposing_faction = nullptr;
	while (opposing_faction)
	{
		int i = Globals::getRandomNumber(0, static_cast<int>(m_factions.size()) - 1);
		if (m_factions[i]->getController() != senderFaction)
		{
			opposing_faction = m_factions[i].get();
		}
	}

	return opposing_faction;
}

bool FactionHandler::removeFaction(eFactionController controller)
{
	const auto faction = GetFaction(m_factions, controller);
	if (faction != m_factions.end())
	{
		m_factions.erase(faction);
		return true;
	}

	return false;
}