#pragma once

#include "Entity.h"
#include "Timer.h"
#include <queue>
#include <functional>

struct IncreaseFactionShieldEvent;
class Laboratory : public Entity
{
public:
	Laboratory(const glm::vec3& startingPosition, Faction& owningFaction);
	~Laboratory();

	int getShieldUpgradeCounter() const;

	void handleEvent(IncreaseFactionShieldEvent gameEvent);
	void update(float deltaTime);
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

private:
	Faction& m_owningFaction;
	int m_shieldUpgradeCounter;
	Timer m_increaseShieldTimer;
};