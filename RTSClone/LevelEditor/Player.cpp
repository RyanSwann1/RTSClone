#include "Player.h"
#include <sstream>

Player::Player(ePlayerType playerType, const glm::vec3& startingHQPosition, const glm::vec3& startingMineralPosition)
	: type(playerType),
	HQ(eModelName::HQ, Globals::convertToNodePosition(startingHQPosition)),
	minerals()
{
	glm::vec3 mineralSpawnPosition = Globals::convertToNodePosition(startingMineralPosition);
	for(auto& mineral : minerals)
	{
		mineral.setModelName(eModelName::Mineral);
		mineral.setPosition(mineralSpawnPosition);
		mineralSpawnPosition.z += Globals::NODE_SIZE;
	}
}

void Player::render(ShaderHandler& shaderHandler) const
{
	for (const auto& mineral : minerals)
	{
		mineral.render(shaderHandler);
	}

	HQ.render(shaderHandler);
}

std::ostream& operator<<(std::ostream& ostream, const Player& player)
{
	switch (player.type)
	{
	case ePlayerType::Human:
		ostream << Globals::TEXT_HEADER_PLAYER;
		break;
	case ePlayerType::AI:
		ostream << Globals::TEXT_HEADER_PLAYERAI;
		break;
	default:
		assert(false);
	}

	ostream << Globals::TEXT_HEADER_HQ;
	ostream << player.HQ.getPosition().x << " " << player.HQ.getPosition().y << " " << player.HQ.getPosition().z << "\n";

	ostream << Globals::TEXT_HEADER_MINERALS;
	for (const auto& mineral : player.minerals)
	{
		ostream << mineral.getPosition().x << " " << mineral.getPosition().y << " " << mineral.getPosition().z << "\n";
	}

	return ostream;
}