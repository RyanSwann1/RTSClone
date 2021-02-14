#include "ProjectileHandler.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent.data.spawnProjectile);
}

void ProjectileHandler::update(float deltaTime, const FactionHandler& factionHandler)
{
	for (auto projectile = m_projectiles.begin(); projectile != m_projectiles.end();)
	{	
		projectile->update(deltaTime);

		const Entity* entity = nullptr;
		if (factionHandler.isFactionActive(projectile->getSenderEvent().targetFaction))
		{
			const Faction& targetFaction = factionHandler.getFaction(projectile->getSenderEvent().targetFaction);
			entity = targetFaction.getEntity(projectile->getAABB(), projectile->getSenderEvent().targetID);
		}

		if (entity || projectile->isReachedDestination())
		{
			if (entity)
			{
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createTakeDamage(projectile->getSenderEvent().senderFaction,
					projectile->getSenderEvent().senderID, projectile->getSenderEvent().senderEntityType, projectile->getSenderEvent().targetFaction,
					projectile->getSenderEvent().targetID, projectile->getSenderEvent().damage));
			}

			projectile = m_projectiles.erase(projectile);
		}
		else
		{
			++projectile;
		}
	}
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