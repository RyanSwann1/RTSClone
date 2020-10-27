#include "Player.h"
#include "LevelFileHandler.h"
#include "ModelManager.h"
#include <sstream>
#include <fstream>
#include <functional>

Player::Player(eFactionController controller)
	: controller(controller),
	HQ(ModelManager::getInstance().getModel(HQ_MODEL_NAME)),
	minerals()
{}

Player::Player(eFactionController factionController, const glm::vec3& hqStartingPosition, const glm::vec3& startingMineralPosition)
	: controller(factionController),
	HQ(ModelManager::getInstance().getModel(HQ_MODEL_NAME), hqStartingPosition),
	minerals()
{
	glm::vec3 mineralSpawnPosition = Globals::convertToNodePosition(startingMineralPosition);
	for (auto& mineral : minerals)
	{
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

#ifdef RENDER_AABB
void Player::renderAABB(ShaderHandler& shaderHandler)
{
	HQ.renderAABB(shaderHandler);
	for (auto& mineral : minerals)
	{
		mineral.renderAABB(shaderHandler);
	}
}
#endif // RENDER_AABB

const std::ifstream& operator>>(std::ifstream& file, Player& player)
{
	auto data = [&player](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string modelName;
		glm::vec3 rotation;
		glm::vec3 position;
		stream >> modelName >> rotation.x >> rotation.y >> rotation.z >> position.x >> position.y >> position.z;
		if (modelName == HQ_MODEL_NAME)
		{
			player.HQ.setRotation(rotation);
			player.HQ.setPosition(position);
			player.HQ.resetAABB();
		}
		else if (modelName == MINERALS_MODEL_NAME)
		{
			int mineralIndex = 0;
			stream >> mineralIndex;
			assert(mineralIndex >= 0 && mineralIndex < player.minerals.size());

			player.minerals[mineralIndex].setPosition(position);
			player.minerals[mineralIndex].setRotation(rotation);
			player.minerals[mineralIndex].resetAABB();
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

	ostream << player.HQ.getModel().modelName << " " <<
		player.HQ.getRotation().x << " " << player.HQ.getRotation().y << " " << player.HQ.getRotation().z << " " <<
		player.HQ.getPosition().x << " " << player.HQ.getPosition().y << " " << player.HQ.getPosition().z << "\n";

	for (int i = 0; i < player.minerals.size(); ++i)
	{
		ostream << player.minerals[i].getModel().modelName << " " <<
			
		player.minerals[i].getRotation().x << " " << player.minerals[i].getRotation().y << " " << 
		player.minerals[i].getRotation().z << " " << 

		player.minerals[i].getPosition().x << " " << player.minerals[i].getPosition().y << " " << 
		player.minerals[i].getPosition().z << " " << i << "\n";
	}

	return ostream;
}