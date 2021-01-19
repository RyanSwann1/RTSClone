#include "Player.h"
#include "LevelFileHandler.h"
#include "ModelManager.h"
#include <sstream>
#include <fstream>
#include <functional>

Player::Player(eFactionController controller)
	: controller(controller),
	HQ(ModelManager::getInstance().getModel(HQ_MODEL_NAME), glm::vec3(0.0)),
	minerals()
{}

Player::Player(eFactionController factionController, const glm::vec3& hqStartingPosition, const glm::vec3& startingMineralPosition)
	: controller(factionController),
	HQ(ModelManager::getInstance().getModel(HQ_MODEL_NAME), hqStartingPosition),
	minerals()
{
	glm::vec3 mineralSpawnPosition = Globals::convertToNodePosition(startingMineralPosition);
	for (int i = 0; i < Globals::MAX_MINERALS_PER_FACTION; ++i)
	{
		minerals.emplace_back(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME), mineralSpawnPosition);
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
			player.minerals.emplace_back(ModelManager::getInstance().getModel(MINERALS_MODEL_NAME), position, rotation);
			player.minerals.back().resetAABB();
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

	for (const auto& mineral : player.minerals)
	{
		ostream << mineral.getModel().modelName << " " <<

		mineral.getRotation().x << " " << mineral.getRotation().y << " " <<
		mineral.getRotation().z << " " <<

		mineral.getPosition().x << " " << mineral.getPosition().y << " " <<
		mineral.getPosition().z << " ";
	}

	return ostream;
}