#include "Level.h"
#include "FactionController.h"
#include "LevelFileHandler.h"
#include "Camera.h"
#include "imgui/imgui.h"
#include "SelectionBox.h"
#include "PlayableAreaDisplay.h"
#include "ModelManager.h"
#include <fstream>

namespace
{
	constexpr glm::vec3 PLAYER_HQ_STARTING_POSITION =
		glm::vec3(4.0f * static_cast<float>(Globals::NODE_SIZE), Globals::GROUND_HEIGHT, 3.0f * static_cast<float>(Globals::NODE_SIZE));

	constexpr glm::vec3 PLAYER_MINERAL_STARTING_POSITION =
		glm::vec3(11.0f * static_cast<float>(Globals::NODE_SIZE), Globals::GROUND_HEIGHT, static_cast<float>(Globals::NODE_SIZE));

	constexpr int MAX_MAP_SIZE = 60 * Globals::NODE_SIZE;
	constexpr int DEFAULT_STARTING_RESOURCES = 100;
	constexpr int DEFAULT_STARTING_POPULATION = 5;
	constexpr glm::ivec2 DEFAULT_MAP_SIZE = { 30, 30 };
	constexpr float ENTITY_TRANSLATE_SPEED = 5.0f;
}

//PlannedEntity
PlannedEntity::PlannedEntity()
	: modelNameIDSelected(0),
	position(),
	modelName(),
	active(false)
{}

//Level
Level::Level(const std::string& levelName)
	: m_levelName(levelName),
	m_plannedEntity(),
	m_translateObject(),
	m_playableAreaDisplay(),
	m_entityManager(),
	m_players(),
	m_selectedPlayer(nullptr),
	m_mapSize(DEFAULT_MAP_SIZE),
	m_factionStartingResources(DEFAULT_STARTING_RESOURCES),
	m_factionStartingPopulation(DEFAULT_STARTING_POPULATION)
{
	m_players.reserve(static_cast<size_t>(eFactionController::Max) + static_cast<size_t>(1));

	if (!LevelFileHandler::loadLevelFromFile(*this))
	{
		m_players.emplace_back(eFactionController::Player, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION);
		m_players.emplace_back(eFactionController::AI_1, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION);

		LevelFileHandler::saveLevelToFile(*this);
	}

	m_playableAreaDisplay.setSize(m_mapSize);
}

std::unique_ptr<Level> Level::create(const std::string& levelName)
{
	if (!LevelFileHandler::isLevelExists(levelName))
	{
		LevelFileHandler::saveLevelName(levelName);
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

int Level::getFactionStartingResources() const
{
	return m_factionStartingResources;
}

int Level::getFactionStartingPopulation() const
{
	return m_factionStartingPopulation;
}

const std::string& Level::getName() const
{
	return m_levelName;
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

void Level::handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime)
{
	switch (currentSFMLEvent.type)
	{
	case sf::Event::KeyPressed:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			m_entityManager.removeAllSelectedEntities();
		}
		break;
	case sf::Event::MouseButtonReleased:
		m_translateObject.deselect();
		break;
	case sf::Event::MouseButtonPressed:
	{
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				glm::vec3 mouseToGroundPosition = { 0.0f, 0.0f, 0.0f };
				if (camera.getMouseToGroundPosition(window, mouseToGroundPosition))
				{
					if (m_translateObject.isSelected(mouseToGroundPosition) &&
						(m_entityManager.isEntitySelected() || m_selectedPlayer))
					{
						m_translateObject.setSelected(true, mouseToGroundPosition);
						sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
					}
					else
					{
						const Entity* entitySelected = m_entityManager.selectEntityAtPosition(mouseToGroundPosition);
						if (entitySelected)
						{
							m_translateObject.setPosition(entitySelected->getPosition());
							m_plannedEntity.active = false;
						}
						else
						{
							if (m_plannedEntity.active)
							{
								m_entityManager.addEntity(m_plannedEntity.modelName, m_plannedEntity.position);
							}
							else
							{
								m_selectedPlayer = nullptr;

								auto player = std::find_if(m_players.begin(), m_players.end(), [&mouseToGroundPosition](const auto& player)
								{
									return player.HQ.getAABB().contains(mouseToGroundPosition);
								});
								if (player != m_players.cend())
								{
									m_translateObject.setPosition(player->HQ.getPosition());
									m_selectedPlayer = &(*player);
								}
							}
						}
					}
				}
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
			{
				m_plannedEntity.active = false;
			}
		}
	}
	case sf::Event::MouseMoved:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
		{
			glm::vec3 mouseToGroundPosition = { 0.0f, 0.0f, 0.0f };
			if (camera.getMouseToGroundPosition(window, mouseToGroundPosition))
			{
				if (m_plannedEntity.active)
				{
					glm::vec3 newPosition = Globals::convertToNodePosition(mouseToGroundPosition);
					AABB AABB(newPosition, ModelManager::getInstance().getModel(m_plannedEntity.modelName));
					if (Globals::isWithinMapBounds(AABB, m_mapSize))
					{
						m_plannedEntity.position = newPosition;
					}
				}
				else if(m_translateObject.isSelected())
				{
					const glm::vec3& position = m_translateObject.getPosition();
					switch (m_translateObject.getCurrentAxisSelected())
					{
					case eAxisCollision::X:
					{
						int xDifference = window.getSize().y / 2 - sf::Mouse::getPosition(window).y;

						m_translateObject.setPosition({ position.x + static_cast<float>(xDifference) * ENTITY_TRANSLATE_SPEED * deltaTime, position.y, position.z });
						sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
					}
						break;
					case eAxisCollision::Z:
					{
						int zDifference = sf::Mouse::getPosition(window).x - window.getSize().x / 2;

						m_translateObject.setPosition({ position.x, position.y, position.z + static_cast<float>(zDifference) * ENTITY_TRANSLATE_SPEED * deltaTime});
						sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
					}
						break;
					}

					Entity* selectedEntity = m_entityManager.getSelectedEntity();
					if (selectedEntity)
					{
						glm::vec3 nodePosition = Globals::convertToNodePosition(m_translateObject.getPosition());
						selectedEntity->setPosition(nodePosition);
						selectedEntity->resetAABB();
					}
					else if (m_selectedPlayer)
					{
						glm::vec3 nodePosition = Globals::convertToNodePosition(m_translateObject.getPosition());
						for (auto& mineral : m_selectedPlayer->minerals)
						{
							mineral.setPosition(mineral.getPosition() + nodePosition - m_selectedPlayer->HQ.getPosition());
							mineral.resetAABB();
						}

						m_selectedPlayer->HQ.setPosition(nodePosition);
						m_selectedPlayer->HQ.resetAABB();
					}
				}
			}
		}
		break;
	}
}

void Level::handleModelNamesGUI()
{
	ImGui::BeginChild("Model Names pane", ImVec2(175, 250), true);

	const auto& modelNames = ModelManager::getInstance().getModelNames();
	assert(m_plannedEntity.modelNameIDSelected >= 0 && m_plannedEntity.modelNameIDSelected < modelNames.size());
	for (int i = 0; i < modelNames.size(); i++)
	{
		if (ImGui::Selectable(modelNames[i].c_str(), m_plannedEntity.modelNameIDSelected == i))
		{
			m_plannedEntity.modelNameIDSelected = i;
			m_plannedEntity.modelName = ModelManager::getInstance().getModelName(modelNames[i]);
			m_plannedEntity.active = true;
		}
	}

	ImGui::EndChild();
}

void Level::handlePlayersGUI()
{
	ImGui::BeginChild("Players Quantity", ImVec2(175, 40), true);

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

				m_players.emplace_back(factionController, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION);
			}
			else if (newPlayerCount < static_cast<int>(m_players.size()))
			{
				m_players.pop_back();
			}
		}
	};

	ImGui::EndChild();
	
	ImGui::BeginChild("Players Details", ImVec2(175, 275), true);
	for (int i = 0; i < static_cast<int>(m_players.size()); ++i)
	{
		if (m_players[i].controller == eFactionController::Player)
		{
			ImGui::Text(std::string("Player " + std::to_string(i)).c_str());
		}
		else
		{
			ImGui::Text(std::string("AI " + std::to_string(i)).c_str());
		}

		glm::vec3 playerPosition = m_players[i].HQ.getPosition();
		if (ImGui::InputFloat("x", &playerPosition.x, Globals::NODE_SIZE) ||
			ImGui::InputFloat("z", &playerPosition.z, Globals::NODE_SIZE))
		{
			for (auto& mineral : m_players[i].minerals)
			{
				glm::vec3 vDifference = playerPosition - m_players[i].HQ.getPosition();
				const glm::vec3& mineralPosition = mineral.getPosition();
				mineral.setPosition({ mineralPosition.x + vDifference.x, mineralPosition.y + vDifference.y, mineralPosition.z + vDifference.z });
				mineral.resetAABB();
			}

			m_players[i].HQ.setPosition(playerPosition);
			m_players[i].HQ.resetAABB();
		}
	}

	ImGui::EndChild();
}

void Level::handleLevelDetailsGUI(bool& showGUIWindow)
{
	ImGui::Begin("Level Details", &showGUIWindow, ImGuiWindowFlags_None);
	if (ImGui::InputInt("x", &m_mapSize.x, 1) ||
		ImGui::InputInt("z", &m_mapSize.y, 1))
	{
		m_mapSize.x = glm::clamp(m_mapSize.x, 0, MAX_MAP_SIZE);
		m_mapSize.y = glm::clamp(m_mapSize.y, 0, MAX_MAP_SIZE);

		m_playableAreaDisplay.setSize(m_mapSize);
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
	if (!LevelFileHandler::saveLevelToFile(*this))
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

	if (m_plannedEntity.active)
	{
		ModelManager::getInstance().getModel(m_plannedEntity.modelName).render(shaderHandler, m_plannedEntity.position);
	}

	m_translateObject.render(shaderHandler, m_entityManager.isEntitySelected() || m_selectedPlayer);
}

void Level::renderPlayableArea(ShaderHandler& shaderHandler) const
{
	m_playableAreaDisplay.render(shaderHandler);
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	m_translateObject.renderAABB(shaderHandler, m_entityManager.isEntitySelected() || m_selectedPlayer);
}
#endif // RENDER_AABB

const std::ifstream& operator>>(std::ifstream& file, Level& level)
{
	assert(file.is_open());

	for (const auto& factionControllerDetails : FACTION_CONTROLLER_DETAILS)
	{
		switch (factionControllerDetails.controller)
		{
		case eFactionController::Player:
		case eFactionController::AI_1:
			assert(LevelFileHandler::isPlayerActive(file, factionControllerDetails.controller));
			level.m_players.emplace_back(factionControllerDetails.controller);
			break;
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			if (LevelFileHandler::isPlayerActive(file, factionControllerDetails.controller))
			{
				level.m_players.emplace_back(factionControllerDetails.controller);
			}
			break;
		default:
			assert(false);
		}
	}

	level.m_mapSize = LevelFileHandler::loadMapSizeFromFile(file);
	level.m_factionStartingResources = LevelFileHandler::loadFactionStartingResources(file);
	level.m_factionStartingPopulation = LevelFileHandler::loadFactionStartingPopulation(file);

	for (auto& player : level.m_players)
	{
		file >> player;
	}

	file >> level.m_entityManager;

	return file;
}