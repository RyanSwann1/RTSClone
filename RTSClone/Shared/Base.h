#pragma once

#include "Mineral.h"
#include <vector>
#include "Quad.h"

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
#ifdef GAME
	Quad quad;
#endif // GAME
};

#ifdef GAME
#include "NonMovable.h"
class ShaderHandler;
class Faction;
struct BaseHandler : private NonCopyable, private NonMovable
{
	BaseHandler(std::vector<Base>&& bases);

	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Base& base, const glm::vec3& position) const;
	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Mineral& mineral, const glm::vec3& position) const;
	const Mineral* getMineral(const glm::vec3& position) const;
	const Base* getBaseAtMineral(const glm::vec3& position) const;
	const Base& getNearestBase(const glm::vec3& position) const;

	void renderMinerals(ShaderHandler& shaderHandler) const;
	void renderBasePositions(ShaderHandler& shaderHandler) const;

	const std::vector<Base> bases;
};
#endif // GAME