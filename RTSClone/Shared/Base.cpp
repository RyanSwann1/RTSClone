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