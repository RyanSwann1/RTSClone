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
	Base();
#endif // LEVEL_EDITOR
	Base(const glm::vec3& position, std::vector<Mineral>&& minerals);
	Base(Base&&) = default;
	Base& operator=(Base&&) = default;

#ifdef GAME
	glm::vec3 getCenteredPosition() const;
	const std::vector<Mineral>& getMinerals() const;
#endif // GAME

#ifdef LEVEL_EDITOR
	void setPosition(const glm::vec3& position);
	Quad quad;
#endif // LEVEL_EDITOR

	glm::vec3 position;
	std::vector<Mineral> minerals;
#ifdef GAME
	Quad quad;
	eFactionController owningFactionController = eFactionController::None;
#endif // GAME
};

#ifdef GAME
class ShaderHandler;
class Faction;
struct GameEvent;
class BaseHandler 
{
public:
	BaseHandler(std::vector<Base>&& m_bases);
	BaseHandler(const BaseHandler&) = delete;
	BaseHandler& operator=(const BaseHandler&) = delete;
	BaseHandler(BaseHandler&&) noexcept = default;
	BaseHandler& operator=(BaseHandler&&) noexcept = default;

	bool isWithinRangeOfMinerals(const glm::vec3& position, float distance) const;
	const std::vector<Base>& getBases() const;
	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Base& base, const glm::vec3& position) const;
	const Mineral* getNearestAvailableMineralAtBase(const Faction& faction, const Mineral& mineral, const glm::vec3& position) const;
	const Mineral* getMineral(const glm::vec3& position) const;
	const Base* getBaseAtMineral(const glm::vec3& position) const;
	const Base* getNearestBase(const glm::vec3& position) const;
	const Base* getNearestUnusedBase(const glm::vec3& position) const;
	const Base* getBase(const glm::vec3& position) const;
	const Base* getBase(const Mineral& mineral) const;

	void handleEvent(const GameEvent& gameEvent);
	void renderMinerals(ShaderHandler& shaderHandler) const;
	void renderBasePositions(ShaderHandler& shaderHandler) const;

private:
	std::vector<Base> m_bases;

	Base& getBase(const glm::vec3& position);
};
#endif // GAME