#include "Level.h"
#include "FactionController.h"
#include "LevelFileHandler.h"
#include "Camera.h"
#include "imgui/imgui.h"
#include "SelectionBox.h"
#include "PlayableAreaDisplay.h"

namespace
{
	void showPlayerDetails(Player& player, const std::string& playerName, const std::string& playerType, int ID)
	{
		ImGui::PushID(ID); // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
		ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
		bool nodeOpen = ImGui::TreeNode(playerName.c_str(), "%s_%u", playerType.c_str(), ID);
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::NextColumn();
		if (nodeOpen)
		{
			ImGui::PushID(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TreeNodeEx("HQ", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			ImGui::NextColumn();
			if (ImGui::InputFloat("x", &player.HQ.getPosition().x, Globals::NODE_SIZE) ||
				ImGui::InputFloat("z", &player.HQ.getPosition().z, Globals::NODE_SIZE))
			{
				player.HQ.resetAABB();
			}
			ImGui::PopID();

			for (int i = 0; i < player.minerals.size(); ++i)
			{
				ImGui::PushID(i + 1);
				ImGui::AlignTextToFramePadding();
				ImGui::TreeNodeEx("Mineral", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "Mineral_%d", i + 1);
				ImGui::NextColumn();
				if (ImGui::InputFloat("x", &player.minerals[i].getPosition().x, Globals::NODE_SIZE) ||
					ImGui::InputFloat("z", &player.minerals[i].getPosition().z, Globals::NODE_SIZE))
				{
					player.minerals[i].resetAABB();
				}
				ImGui::PopID();
			}

			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	constexpr std::array<glm::vec3, static_cast<size_t>(eFactionController::Max) + 1> PLAYER_HQ_STARTING_POSITIONS =
	{
		glm::vec3(35.0f, Globals::GROUND_HEIGHT, 15.0f),
		glm::vec3(35.0f, Globals::GROUND_HEIGHT, 150.0f),
		glm::vec3(135.0f, Globals::GROUND_HEIGHT, 150.0f),
		glm::vec3(135.0f, Globals::GROUND_HEIGHT, 15.0f)
	};
	constexpr std::array<glm::vec3, static_cast<size_t>(eFactionController::Max) + 1> PLAYER_MINERAL_STARTING_POSITIONS =
	{
		glm::vec3(70.0f, Globals::GROUND_HEIGHT, Globals::NODE_SIZE),
		glm::vec3(70.0f, Globals::GROUND_HEIGHT, 150.0f),
		glm::vec3(170.0f, Globals::GROUND_HEIGHT, 150.0f),
		glm::vec3(170.0f, Globals::GROUND_HEIGHT, Globals::NODE_SIZE)
	};

	constexpr int MAX_MAP_SIZE = 60 * Globals::NODE_SIZE;
	constexpr int DEFAULT_STARTING_RESOURCES = 100;
	constexpr int DEFAULT_STARTING_POPULATION = 5;
	constexpr glm::ivec2 DEFAULT_MAP_SIZE = { 30, 30 };
}

Level::Level(const std::string& levelName)
	: m_levelName(levelName),
	m_entityManager(),
	m_players(),
	m_mapSize(DEFAULT_MAP_SIZE),
	m_factionStartingResources(DEFAULT_STARTING_RESOURCES),
	m_factionStartingPopulation(DEFAULT_STARTING_POPULATION)
{
	m_players.reserve(static_cast<size_t>(eFactionController::Max) + static_cast<size_t>(1));

	if (!LevelFileHandler::loadLevelFromFile(m_levelName, m_entityManager, m_players, m_mapSize,
		m_factionStartingResources, m_factionStartingPopulation))
	{
		m_players.emplace_back(eFactionController::Player, PLAYER_HQ_STARTING_POSITIONS[static_cast<int>(eFactionController::Player)],
			PLAYER_MINERAL_STARTING_POSITIONS[static_cast<int>(eFactionController::Player)]);

		m_players.emplace_back(eFactionController::AI_1, PLAYER_HQ_STARTING_POSITIONS[static_cast<int>(eFactionController::AI_1)],
			PLAYER_MINERAL_STARTING_POSITIONS[static_cast<int>(eFactionController::AI_1)]);

		LevelFileHandler::saveLevelToFile(m_levelName, m_entityManager, m_players, m_mapSize, m_factionStartingResources, m_factionStartingPopulation);
	}
}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	if (!LevelFileHandler::isLevelExists(levelName))
	{
		return std::unique_ptr<Level>(new Level(levelName));
	}

	return std::unique_ptr<Level>();
}

std::unique_ptr<Level> Level::load(const std::string& levelName)
{
	if (LevelFileHandler::isLevelExists(levelName))
	{
		return std::unique_ptr<Level>(new Level(levelName));
	}

	return std::unique_ptr<Level>();
}

const std::vector<Player>& Level::getPlayers() const
{
	return m_players;
}

const glm::ivec2& Level::getMapSize() const
{
	return m_mapSize;
}

const EntityManager& Level::getEntityManager() const
{
	return m_entityManager;
}

void Level::addEntity(eModelName modelName, const glm::vec3& position)
{
	m_entityManager.addEntity(modelName, position);
}

void Level::handleInput(const sf::Event& currentSFMLEvent, const SelectionBox& selectionBox, const Camera& camera,
	bool plannedEntityActive, const sf::Window& window, const Entity& plannedEntity)
{
	switch (currentSFMLEvent.type)
	{
	case sf::Event::KeyPressed:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			m_entityManager.removeAllSelectedEntities();
		}
		break;
	case sf::Event::MouseButtonPressed:
	{
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
				bool entitySelected = m_entityManager.selectEntityAtPosition(mouseToGroundPosition);
				if (plannedEntityActive && !entitySelected)
				{
					m_entityManager.addEntity(plannedEntity.getModelName(), plannedEntity.getPosition());
				}
			}
		}
	}
	case sf::Event::MouseMoved:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
		{
			if (selectionBox.isActive() && selectionBox.isMinimumSize())
			{
				m_entityManager.selectEntities(selectionBox);
			}
		}
		break;
	}
}

void Level::handlePlayerDetails(bool& showDetailsWindow)
{
	ImGui::SetNextWindowPos(ImVec2(700, 700), ImGuiCond_FirstUseEver);
	ImGui::Begin("Players", &showDetailsWindow, ImGuiWindowFlags_None);
	ImGui::BeginChild("Players One");

	int newPlayerCount = static_cast<int>(m_players.size());
	if (ImGui::InputInt("Player Amount", &newPlayerCount, 1, ImGuiInputTextFlags_ReadOnly))
	{
		if (newPlayerCount >= Globals::MIN_FACTIONS && newPlayerCount <= Globals::MAX_FACTIONS)
		{
			if (newPlayerCount > static_cast<int>(m_players.size()))
			{
				eFactionController factionController;
				switch (newPlayerCount)
				{
				case Globals::MAX_FACTIONS - Globals::MIN_FACTIONS / 2:
					factionController = eFactionController::AI_2;
					break;
				case Globals::MAX_FACTIONS:
					factionController = eFactionController::AI_3;
					break;
				default:
					assert(false);
				}

				m_players.emplace_back(factionController, PLAYER_HQ_STARTING_POSITIONS[newPlayerCount - 1],
					PLAYER_MINERAL_STARTING_POSITIONS[newPlayerCount - 1]);
			}
			else if (newPlayerCount < static_cast<int>(m_players.size()))
			{
				m_players.pop_back();
			}
		}
	};

	for (int i = 0; i < static_cast<int>(m_players.size()); ++i)
	{
		if (i == 0)
		{
			showPlayerDetails(m_players[i], "Player", "Human", i);

		}
		else
		{
			showPlayerDetails(m_players[i], "Player", "AI", i);
		}
	}

	ImGui::EndChild();
	ImGui::End();
}

void Level::handleLevelDetails(bool& showDetailsWindow, PlayableAreaDisplay& playableAreaDisplay)
{
	ImGui::Begin("Level Details", &showDetailsWindow, ImGuiWindowFlags_None);
	if (ImGui::InputInt("x", &m_mapSize.x, 1) ||
		ImGui::InputInt("z", &m_mapSize.y, 1))
	{
		m_mapSize.x = glm::clamp(m_mapSize.x, 0, MAX_MAP_SIZE);
		m_mapSize.y = glm::clamp(m_mapSize.y, 0, MAX_MAP_SIZE);

		playableAreaDisplay.setSize(m_mapSize);
	}
	ImGui::Text("Starting Resources");
	if (ImGui::InputInt("Resources", &m_factionStartingResources, 5))
	{
		if (m_factionStartingResources < Globals::WORKER_RESOURCE_COST)
		{
			m_factionStartingResources = Globals::WORKER_RESOURCE_COST;
		}
	}
	ImGui::Text("Starting Population");
	if (ImGui::InputInt("Population", &m_factionStartingPopulation, 1))
	{
		if (m_factionStartingPopulation < Globals::WORKER_POPULATION_COST)
		{
			m_factionStartingPopulation = Globals::WORKER_POPULATION_COST;
		}
	}
	ImGui::End();
}

void Level::save() const
{
	if (!LevelFileHandler::saveLevelToFile(m_levelName, m_entityManager, m_players, m_mapSize,
		 m_factionStartingResources, m_factionStartingPopulation))
	{
		std::cout << "Unable to save file " + m_levelName << "\n";
	}
}

void Level::render(ShaderHandler& shaderHandler) const
{
	m_entityManager.render(shaderHandler);

	for (auto& player : m_players)
	{
		player.render(shaderHandler);
	}
}