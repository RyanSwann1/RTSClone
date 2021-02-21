#pragma once

#include "Mineral.h"
#include <vector>
#include "Quad.h"
#ifdef GAME
#include "FactionController.h"
#endif // GAME

struct Base
{
#ifdef LEVEL_EDITOR
	Base(const glm::vec3& position);
#endif // LEVEL_EDITOR
	Base(const glm::vec3& position, std::vector<Mineral>&& minerals);

#ifdef GAME
	glm::vec3 getConvertedPosition() const;
#endif // GAME

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
	Quad quad;
#endif // LEVEL_EDITOR

	glm::vec3 position;
	std::vector<Mineral> minerals;
#ifdef GAME
	Quad quad;
	eFactionController owningFactionController;
#endif // GAME
};

#ifdef GAME
#include "NonMovable.h"
class ShaderHandler;
class Faction;
struct GameEvent;
class BaseHandler : private NonCopyable, private NonMovable
{
public:
	BaseHandler(std::vector<Base>&& m_bases);

	bool isWithinRangeOfMinerals(const glm::vec3& position, float distance) const;
	const std::vector<Base>& getBases() const;
	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Base& base, const glm::vec3& position) const;
	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Mineral& mineral, const glm::vec3& position) const;
	const Mineral* getMineral(const glm::vec3& position) const;
	const Base* getBaseAtMineral(const glm::vec3& position) const;
	const Base& getNearestBase(const glm::vec3& position) const;

	void handleEvent(const GameEvent& gameEvent);
	void renderMinerals(ShaderHandler& shaderHandler) const;
	void renderBasePositions(ShaderHandler& shaderHandler) const;

private:
	std::vector<Base> m_bases;
};
#endif // GAME