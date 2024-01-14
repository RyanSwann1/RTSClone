#include "Base.h"
#include "Globals.h"
#include "GameEvents.h"
#ifdef GAME
#include "Factions/Faction.h"
#include <limits>
#endif // GAME

namespace
{
	const glm::vec3 MAIN_BASE_QUAD_COLOR = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 MAIN_BASE_QUAD_SIZE = { Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE };
#ifdef GAME
	const float QUAD_OPACITY = 0.25f;
#endif // GAME
}

#ifdef GAME
Base::Base(const glm::vec3& position, std::vector<Mineral>&& minerals)
	: position(position),
	minerals(std::move(minerals)),
	quad(position, MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR, QUAD_OPACITY),
	owningFactionController(eFactionController::None)
{}

glm::vec3 Base::getCenteredPosition() const
{
	return Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position));
}

const std::vector<Mineral>& Base::getMinerals() const
{
	return minerals;
}

#endif // GAME

#ifdef LEVEL_EDITOR
Base::Base()
	: quad(glm::vec3(0.f), MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR),
	position(0.f),
	minerals(Globals::MAX_MINERALS)
{}

Base::Base(const glm::vec3& position, std::vector<Mineral>&& minerals)
	: quad(position, MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR),
	position(position),
	minerals(std::move(minerals))
{}

void Base::setPosition(const glm::vec3 & _position)
{
	glm::vec3 oldPosition = position;
	position = _position;
	quad.setPosition(position);

	for (auto& mineral : minerals)
	{
		mineral.setPosition(position + (mineral.getPosition() - oldPosition));
	}
}
#endif // LEVEL_EDITOR

#ifdef GAME
namespace
{
	std::array<const Mineral*, Globals::MAX_MINERALS> getClosestMinerals(const std::vector<Mineral>& minerals, const glm::vec3& position)
	{
		assert(static_cast<int>(minerals.size()) == Globals::MAX_MINERALS);
		std::array<const Mineral*, Globals::MAX_MINERALS> sortedMinerals;
		for (int i = 0; i < static_cast<int>(minerals.size()); ++i)
		{
			sortedMinerals[i] = &minerals[i];
		}

		std::sort(sortedMinerals.begin(), sortedMinerals.end(), [&position](const auto& mineralA, const auto& mineralB)
		{
			assert(mineralA && mineralB);
			return Globals::getSqrDistance(mineralA->getPosition(), position) <
				Globals::getSqrDistance(mineralB->getPosition(), position);
		});

		return sortedMinerals;
	}
}

BaseHandler::BaseHandler(std::vector<Base>&& m_bases)
	: m_bases(std::move(m_bases))
{}

bool BaseHandler::isWithinRangeOfMinerals(const glm::vec3& position, float distance) const
{
	for (const auto& base : m_bases)
	{
		for (const auto& mineral : base.minerals)
		{
			if (Globals::getSqrDistance(mineral.getPosition(), position) <= distance * distance)
			{
				return true;
			}
		}
	}
	
	return false;
}

const std::vector<Base>& BaseHandler::getBases() const
{
	return m_bases;
}

const Mineral* BaseHandler::getNearestAvailableMineralAtBase(const Faction& faction, const Base& base,
	const glm::vec3& position) const
{
	std::array<const Mineral*, Globals::MAX_MINERALS> minerals = getClosestMinerals(base.minerals, position);
	for (const auto& mineral : minerals)
	{
		assert(mineral);
		if (!faction.IsMineralReachedHarvestingCapacity(*mineral))
		{
			return mineral;
		}
	}

	return nullptr;
}

const Mineral* BaseHandler::getNearestAvailableMineralAtBase(const Faction& faction, const Mineral& _mineral,
	const glm::vec3& position) const
{
	assert(faction.IsMineralReachedHarvestingCapacity(_mineral));
	if (const Base* base = getBase(_mineral))
	{
		std::array<const Mineral*, Globals::MAX_MINERALS> minerals = getClosestMinerals(base->minerals, position);
		for (const auto& mineral : minerals)
		{
			assert(mineral);
			if (&(*mineral) != &_mineral &&
				!faction.IsMineralReachedHarvestingCapacity(*mineral))
			{
				return mineral;
			}
		}
	}

	return nullptr;
}

const Mineral* BaseHandler::getMineral(const glm::vec3 & position) const
{
	for (const auto& base : m_bases)
	{
		for (const auto& mineral : base.minerals)
		{
			if (mineral.getAABB().contains(position))
			{
				return &mineral;
			}
		}
	}

	return nullptr;
}

const Base* BaseHandler::getBaseAtMineral(const glm::vec3& position) const
{
	for (const auto& base : m_bases)
	{
		for (const auto& mineral : base.minerals)
		{
			if (mineral.getAABB().contains(position))
			{
				return &base;
			}
		}
	}

	return nullptr;
}

const Base* BaseHandler::getNearestBase(const glm::vec3& position) const
{
	const Base* closestBase = nullptr;
	std::for_each(m_bases.cbegin(), m_bases.cend(), 
		[&, closestDistance = std::numeric_limits<float>::max()](const auto& base) mutable
	{
		float distance = Globals::getSqrDistance(base.position, position);
		if (distance < closestDistance)
		{
			closestBase = &base;
			closestDistance = distance;
		}
	});

	return closestBase;
}

const Base* BaseHandler::getNearestUnusedBase(const glm::vec3& position) const
{
	float closestDistance = std::numeric_limits<float>::max();
	const Base* closestBase = nullptr;
	for (const auto& base : m_bases)
	{
		float distance = Globals::getSqrDistance(base.position, position);
		if (distance < closestDistance && 
			base.owningFactionController == eFactionController::None)
		{
			closestBase = &base;
			closestDistance = distance;
		}
	}

	return closestBase;
}

const Base* BaseHandler::getBase(const glm::vec3& position) const
{
	auto base = std::find_if(m_bases.cbegin(), m_bases.cend(), [&position](const auto& base)
	{
		return base.getCenteredPosition() == position;	
	});
	if (base != m_bases.cend())
	{
		return &(*base);
	}

	return nullptr;
}

const Base* BaseHandler::getBase(const Mineral& _mineral) const
{
	for (const auto& base : m_bases)
	{
		for (const auto& mineral : base.minerals)
		{
			if (&mineral == &_mineral)
			{
				return &base;
			}
		}
	}

	return nullptr;
}

Base& BaseHandler::getBase(const glm::vec3& position)
{
	auto base = std::find_if(m_bases.begin(), m_bases.end(), [&position](const auto& base)
	{
		return base.getCenteredPosition() == position;
	});
	assert(base != m_bases.end());
	return (*base);
}

void BaseHandler::handleEvent(const GameEvent& gameEvent)
{
	switch (gameEvent.type)
	{
	case eGameEventType::AttachFactionToBase:
	{
		Base& base = getBase(gameEvent.data.attachFactionToBase.position);
		assert(base.owningFactionController == eFactionController::None);
		base.owningFactionController = gameEvent.data.attachFactionToBase.factionController;
	}
		break;
	case eGameEventType::DetachFactionFromBase:
	{
		Base& base = getBase(gameEvent.data.attachFactionToBase.position);
		assert(base.owningFactionController == gameEvent.data.detachFactionFromBase.factionController);
		base.owningFactionController = eFactionController::None;
	}
		break;
	}
}

void BaseHandler::renderMinerals(ShaderHandler& shaderHandler) const
{
	for (const auto& base : m_bases)
	{
		for (const auto& mineral : base.minerals)
		{
			mineral.render(shaderHandler);
		}
	}
}

void BaseHandler::renderBasePositions(ShaderHandler& shaderHandler) const
{
	for (const auto& base : m_bases)
	{
		if (base.owningFactionController == eFactionController::None)
		{
			base.quad.render(shaderHandler);
		}
	}
}
#endif // GAME