#pragma once

#include "Mineral.h"
#include <vector>
#ifdef LEVEL_EDITOR
#include "Quad.h"
#endif // LEVEL_EDITOR

struct Base
{
#ifdef LEVEL_EDITOR
	Base(const glm::vec3& position);
#endif // LEVEL_EDITOR
	Base(const glm::vec3& position, std::vector<Mineral>&& minerals);

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
	Quad quad;
#endif // LEVEL_EDITOR

	glm::vec3 position;
	std::vector<Mineral> minerals;
};

#ifdef GAME
#include "NonMovable.h"
#include <list>
class ShaderHandler;
class Faction;
struct BaseHandler : private NonCopyable, private NonMovable
{
	BaseHandler(std::vector<Base>&& bases);

	const Mineral* getAvailableMineralAtBase(const Faction& faction, const Mineral& mineral) const;
	const Mineral* getMineral(const glm::vec3& position) const;
	const Base* getBaseAtMineral(const glm::vec3& position) const;
	void render(ShaderHandler& shaderHandler) const;

	const std::vector<Base> bases;
};
#endif // GAME