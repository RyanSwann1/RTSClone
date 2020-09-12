#include "Player.h"

Player::Player(const glm::vec3& startingHQPosition, const glm::vec3& startingMineralPosition)
	: m_minerals(),
	m_HQ(eModelName::HQ, Globals::convertToNodePosition(startingHQPosition))
{
	glm::vec3 mineralSpawnPosition = Globals::convertToNodePosition(startingMineralPosition);
	for(auto& mineral : m_minerals)
	{
		mineral.setModelName(eModelName::Mineral);
		mineral.setPosition(mineralSpawnPosition);
		mineralSpawnPosition.z += Globals::NODE_SIZE;
	}
}

void Player::render(ShaderHandler& shaderHandler) const
{
	for (const auto& mineral : m_minerals)
	{
		mineral.render(shaderHandler);
	}

	m_HQ.render(shaderHandler);
}