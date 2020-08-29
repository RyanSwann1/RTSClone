#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Projectile.h"
#include <vector>

class FactionAI;
class FactionPlayer;
class ShaderHandler;
class ProjectileHandler : private NonCopyable, private NonMovable
{
public:
	ProjectileHandler();
	
	void addProjectile(const GameEvent& gameEvent);
	void update(float deltaTime, const FactionPlayer& player, const FactionAI& playerAI);
	void render(ShaderHandler& shaderHandler);

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Projectile> m_projectiles;
};