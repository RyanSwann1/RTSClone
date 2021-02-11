#include "Base.h"
#include "Globals.h"
#ifdef GAME
#include "Faction.h"
#include "GameEventHandler.h"
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

glm::vec3 Base::getConvertedPosition() const
{
	return Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position));
}
#endif // GAME

#ifdef LEVEL_EDITOR
Base::Base(const glm::vec3& position)
	: quad(position, MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR),
	position(position),
	minerals(Globals::MAX_MINERALS)
{}

Base::Base(const glm::vec3& position, std::vector<Mineral>&& minerals)
	: quad(position, MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR),
	position(position),
	minerals(std::move(minerals))
{}

void Base::setPosition(const glm::vec3 & _position)
{
	position = _position;
	quad.setPosition(position);

	for (auto& mineral : minerals)
	{
		mineral.setPosition(position);
	}
}
#endif // LEVEL_EDITOR

#ifdef GAME
namespace
{
	const Base& getBase(const std::vector<Base>& bases, const Mineral& _mineral)
	{
		for (const auto& base : bases)
		{
			for (const auto& mineral : base.minerals)
			{
				if (&mineral == &_mineral)
				{
					return base;
				}
			}
		}

		assert(false);
	}

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

	Base& getBase(std::vector<Base>& bases, const glm::vec3& position)
	{
		auto base = std::find_if(bases.begin(), bases.end(), [&position](const auto& base)
		{
			return base.getConvertedPosition() == position;
		});
		assert(base != bases.end());

		return (*base);
	}
}

BaseHandler::BaseHandler(std::vector<Base>&& bases)
	: bases(std::move(bases))
{}

const Mineral* BaseHandler::getNearestAvailableMineralAtBase(const Faction& faction, const Base& base, 
	const glm::vec3& position) const
{
	std::array<const Mineral*, Globals::MAX_MINERALS> minerals = getClosestMinerals(base.minerals, position);
	for (const auto& mineral : minerals)
	{
		assert(mineral);
		if (!faction.isMineralInUse(*mineral))
		{
			return mineral;
		}
	}

	return nullptr;
}

const Mineral* BaseHandler::getNearestAvailableMineralAtBase(const Faction& faction, const Mineral& _mineral,
	const glm::vec3& position) const
{
	assert(faction.isMineralInUse(_mineral));
	std::array<const Mineral*, Globals::MAX_MINERALS> minerals = getClosestMinerals(getBase(bases, _mineral).minerals, position);
	for (const auto& mineral : minerals)
	{
		assert(mineral);
		if (&(*mineral) != &_mineral &&
			!faction.isMineralInUse(*mineral))
		{
			return mineral;
		}
	}

	return nullptr;
}

const Mineral* BaseHandler::getMineral(const glm::vec3 & position) const
{
	for (const auto& base : bases)
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
	for (const auto& base : bases)
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

const Base& BaseHandler::getNearestBase(const glm::vec3& position) const
{
	float closestDistance = std::numeric_limits<float>::max();
	const Base* closestBase = nullptr;
	for (const auto& base : bases)
	{
		float distance = Globals::getSqrDistance(base.position, position);
		if (distance < closestDistance)
		{
			closestBase = &base;
			closestDistance = distance;
		}
	}

	assert(closestBase);
	return *closestBase;
}

void BaseHandler::handleEvent(const GameEvent& gameEvent)
{
	switch (gameEvent.type)
	{
	case eGameEventType::AttachFactionToBase:
	{
		Base& base = getBase(bases, gameEvent.data.attachFactionToBase.position);
		assert(base.owningFactionController == eFactionController::None);
		base.owningFactionController = gameEvent.data.attachFactionToBase.factionController;
	}
		break;
	}
}

void BaseHandler::renderMinerals(ShaderHandler& shaderHandler) const
{
	for (const auto& base : bases)
	{
		for (const auto& mineral : base.minerals)
		{
			mineral.render(shaderHandler);
		}
	}
}

void BaseHandler::renderBasePositions(ShaderHandler& shaderHandler) const
{
	for (const auto& base : bases)
	{
		base.quad.render(shaderHandler);
	}
}
#endif // GAME