#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Projectile.h"
#include <vector>

class Faction;
class ShaderHandler;
class ProjectileHandler : private NonCopyable, private NonMovable
{
public:
	ProjectileHandler();
	
	void addProjectile(const GameEvent& gameEvent);
	void update(float deltaTime, const Faction& player, const Faction& playerAI);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Projectile> m_projectiles;
};