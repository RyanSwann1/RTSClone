#include "Player.h"
#include "LevelFileHandler.h"
#include <sstream>
#include <fstream>
#include <functional>

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

const std::ifstream& operator>>(std::ifstream& file, Player& player)
{
	auto data = [&player](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string rawModelName;
		glm::vec3 position;
		stream >> rawModelName >> position.x >> position.y >> position.z;
		switch (static_cast<eModelName>(std::stoi(rawModelName)))
		{
		case eModelName::HQ:
			player.HQ.setPosition(position);
			break;
		case eModelName::Mineral:
		{
			int mineralIndex = -1;
			stream >> mineralIndex;
			assert(mineralIndex >= 0 && mineralIndex < player.minerals.size());

			player.minerals[mineralIndex].setPosition(position);
		}
		break;
		}
	};

	ePlayerType playerType = player.type;
	auto conditional = [playerType](const std::string& line)
	{
		return (playerType == ePlayerType::Human && line == Globals::TEXT_HEADER_PLAYER) ||
			(playerType == ePlayerType::AI && line == Globals::TEXT_HEADER_PLAYERAI);
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return file;
}

std::ostream& operator<<(std::ostream& ostream, const Player& player)
{
	switch (player.type)
	{
	case ePlayerType::Human:
		ostream << Globals::TEXT_HEADER_PLAYER << "\n";
		break;
	case ePlayerType::AI:
		ostream << Globals::TEXT_HEADER_PLAYERAI << "\n";
		break;
	default:
		assert(false);
	}

	ostream << static_cast<int>(player.HQ.getModelName()) << " " <<  
		player.HQ.getPosition().x << " " << player.HQ.getPosition().y << " " << player.HQ.getPosition().z << "\n";

	for (int i = 0; i < player.minerals.size(); ++i)
	{
		ostream << static_cast<int>(player.minerals[i].getModelName()) << " " <<
			player.minerals[i].getPosition().x << " " << player.minerals[i].getPosition().y << " " << 
			player.minerals[i].getPosition().z << " " << i << "\n";
	}

	return ostream;
}