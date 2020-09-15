#include "Player.h"
#include "LevelFileHandler.h"
#include <sstream>
#include <fstream>
#include <functional>

Player::Player(eFactionController controller)
	: controller(controller),
	HQ(),
	minerals()
{
	HQ.setModelName(eModelName::HQ); //TODO: Change to constructor
	for (auto& mineral : minerals)
	{
		mineral.setModelName(eModelName::Mineral);
	}
}

Player::Player(eFactionController factionController, const glm::vec3& hqStartingPosition, const glm::vec3& startingMineralPosition)
	: controller(factionController),
	HQ(eModelName::HQ, hqStartingPosition),
	minerals()
{
	glm::vec3 mineralSpawnPosition = Globals::convertToNodePosition(startingMineralPosition);
	for (auto& mineral : minerals)
	{
		mineral.setModelName(eModelName::Mineral);
		mineral.setPosition(mineralSpawnPosition);
		mineralSpawnPosition.z += Globals::NODE_SIZE;
	}
}

Player::Player(Player&& orig) noexcept
	: controller(orig.controller),
	HQ(std::move(orig.HQ)),
	minerals(std::move(orig.minerals))
{}

Player& Player::operator=(Player&& orig) noexcept
{
	controller = orig.controller;
	HQ = std::move(orig.HQ);
	minerals = std::move(orig.minerals);

	return *this;
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
			int mineralIndex = 0;
			stream >> mineralIndex;
			assert(mineralIndex >= 0 && mineralIndex < player.minerals.size());

			player.minerals[mineralIndex].setPosition(position);
		}
		break;
		}
	};

	eFactionController factionController = player.controller;
	auto conditional = [factionController](const std::string& line)
	{
		return line == FACTION_CONTROLLER_DETAILS[static_cast<int>(factionController)].text;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return file;
}

std::ostream& operator<<(std::ostream& ostream, const Player& player)
{
	ostream << FACTION_CONTROLLER_DETAILS[static_cast<int>(player.controller)].text << "\n";

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