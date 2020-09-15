#include "ProjectileHandler.h"
#include "Faction.h"
#include "GameEventHandler.h"

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent);
}

void ProjectileHandler::update(float deltaTime, const std::vector<std::unique_ptr<Faction>>& factions)
{
	for (auto projectile = m_projectiles.begin(); projectile != m_projectiles.end();)
	{	
		projectile->update(deltaTime);
	
		bool projectileCollision = false;
		eFactionController targetFactionController = projectile->getSenderEvent().targetFaction;
		auto targetFaction = std::find_if(factions.begin(), factions.end(), [targetFactionController](const auto& faction)
		{
			return faction->getController() == targetFactionController;
		});
		assert(targetFaction != factions.end());

		projectileCollision = targetFaction->get()->getEntity(projectile->getAABB(), projectile->getSenderEvent().targetID);

		if (projectileCollision || projectile->isReachedDestination())
		{
			if (projectileCollision)
			{
				GameEventHandler::getInstance().addEvent({ eGameEventType::Attack, projectile->getSenderEvent().senderFaction,
					projectile->getID(), projectile->getSenderEvent().targetFaction, projectile->getSenderEvent().targetID });
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