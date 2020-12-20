#pragma once

#include "Entity.h"
#include "Timer.h"
#include <queue>
#include <functional>

class Laboratory : public Entity
{
public:
	Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction);
	~Laboratory();

	void addIncreaseShieldCommand(const std::function<void()>& command);
	void update(float deltaTime);
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

private:
	std::queue<std::function<void()>> m_increaseShieldCommands; //Faction::increaseShield
	Timer m_increaseShieldTimer;
};