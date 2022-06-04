#pragma once

#include "Entity.h"
#include "Core/Timer.h"
#include <queue>
#include <functional>

struct IncreaseFactionShieldEvent;
class Laboratory : public Entity
{
public:
	Laboratory(const Position& position, Faction& owningFaction);
	Laboratory(Laboratory&&) = default;
	Laboratory& operator=(Laboratory&&) = default;
	~Laboratory();

	int getShieldUpgradeCounter() const;
	bool is_group_selectable() const override;

	void handleEvent(IncreaseFactionShieldEvent gameEvent);
	void update(float deltaTime);
	void render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const override;

private:
	std::reference_wrapper<Faction> m_owningFaction;
	int m_shieldUpgradeCounter;
	Timer m_increaseShieldTimer;
};