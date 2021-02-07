#include "Base.h"
#include "Globals.h"

namespace
{
	const glm::vec3 MAIN_BASE_QUAD_COLOR = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 MAIN_BASE_QUAD_SIZE = { Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE };
}

#ifdef GAME
Base::Base(const glm::vec3& position, std::vector<Mineral>&& minerals)
	: position(position),
	minerals(std::move(minerals))
{}
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
BaseHandler::BaseHandler(std::vector<Base>&& bases)
	: bases(std::move(bases))
{}

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

void BaseHandler::render(ShaderHandler& shaderHandler) const
{
	for (const auto& base : bases)
	{
		for (const auto& mineral : base.minerals)
		{
			mineral.render(shaderHandler);
		}
	}
}
#endif // GAME