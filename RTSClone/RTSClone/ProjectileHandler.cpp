#include "ProjectileHandler.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"

namespace
{
	bool isHitEntity(const Projectile& projectile, const FactionHandler& factionHandler)
	{
		const Entity* entity = nullptr;
		if (factionHandler.isFactionActive(projectile.getSenderEvent().targetFaction))
		{
			const Faction& targetFaction = factionHandler.getFaction(projectile.getSenderEvent().targetFaction);
			entity = targetFaction.getEntity(projectile.getAABB(), projectile.getSenderEvent().targetID, projectile.getSenderEvent().targetEntityType);
		}

		return entity;
	}
}

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent.data.spawnProjectile);
}

void ProjectileHandler::update(float deltaTime, const FactionHandler& factionHandler)
{
	std::for_each(m_projectiles.begin(), m_projectiles.end(), [deltaTime](auto& projectile)
	{
		projectile.update(deltaTime);
	});

	auto iter = std::remove_if(m_projectiles.begin(), m_projectiles.end(), [&factionHandler](auto& projectile)
	{
		bool hitEntity = isHitEntity(projectile, factionHandler);
		if (hitEntity)
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::createTakeDamage(projectile.getSenderEvent().senderFaction,
				projectile.getSenderEvent().senderID, projectile.getSenderEvent().senderEntityType, projectile.getSenderEvent().targetFaction,
				projectile.getSenderEvent().targetID, projectile.getSenderEvent().damage));
		}
		return hitEntity || projectile.isReachedDestination();
	});
	m_projectiles.erase(iter, m_projectiles.end());
}

void ProjectileHandler::render(ShaderHandler& shaderHandler) const
{
	for (const auto& projectile : m_projectiles)
	{
		projectile.render(shaderHandler);
	}
}

#ifdef RENDER_AABB
void ProjectileHandler::renderAABB(ShaderHandler& shaderHandler)
{
	for (auto& projectile : m_projectiles)
	{
		projectile.renderAABB(shaderHandler);
	}
}
#endif // RENDER_AABB