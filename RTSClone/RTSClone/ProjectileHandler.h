#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Projectile.h"
#include <vector>
#include <memory>

class ShaderHandler;
class FactionHandler;
class ProjectileHandler : private NonCopyable, private NonMovable
{
public:
	ProjectileHandler();
	
	void addProjectile(const GameEvent& gameEvent);
	void update(float deltaTime, const FactionHandler& factionHandler);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Projectile> m_projectiles;
};