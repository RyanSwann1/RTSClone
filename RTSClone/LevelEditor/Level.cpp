#include "Level.h"
#include "FactionController.h"
#include "LevelFileHandler.h"
#include "Camera.h"
#include "imgui/imgui.h"
#include "SelectionBox.h"
#include "ModelManager.h"
#include "glad.h"
#include "ShaderHandler.h"
#include <fstream>

namespace
{
	const glm::vec3 PLAYER_HQ_STARTING_POSITION =
		glm::vec3(4.0f * static_cast<float>(Globals::NODE_SIZE), Globals::GROUND_HEIGHT, 3.0f * static_cast<float>(Globals::NODE_SIZE));

	const glm::vec3 PLAYER_MINERAL_STARTING_POSITION =
		glm::vec3(11.0f * static_cast<float>(Globals::NODE_SIZE), Globals::GROUND_HEIGHT, static_cast<float>(Globals::NODE_SIZE));

	const int MAX_MAP_SIZE = 60 * Globals::NODE_SIZE;
	const int DEFAULT_STARTING_RESOURCES = 100;
	const int DEFAULT_STARTING_POPULATION_CAP = 5;
	const glm::ivec2 DEFAULT_MAP_SIZE = { 30, 30 };
	const float ENTITY_TRANSLATE_SPEED = 5.0f;
	const glm::vec3 PLAYABLE_AREA_GROUND_COLOR = { 1.0f, 1.0f, 0.5f };
	const float PLAYABLE_AREA_OPACITY = 0.1f;
}

//PlannedEntity
PlannedEntity::PlannedEntity()
	: modelNameIDSelected(0),
	position(),
	model(nullptr)
{}

//Level
Level::Level(const std::string& levelName)
	: m_levelName(levelName),
	m_plannedEntity(),
	m_translateObject(),
	m_size(DEFAULT_MAP_SIZE),
	m_playableArea(),
	m_gameObjectManager(),
	m_players(),
	m_selectedPlayer(nullptr),
	m_factionStartingResources(DEFAULT_STARTING_RESOURCES),
	m_factionStartingPopulationCap(DEFAULT_STARTING_POPULATION_CAP)
{
	m_players.reserve(static_cast<size_t>(eFactionController::Max) + static_cast<size_t>(1));

	if (!LevelFileHandler::loadLevelFromFile(*this))
	{
		m_players.emplace_back(std::make_unique<Player>(eFactionController::Player, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION));
		m_players.emplace_back(std::make_unique<Player>(eFactionController::AI_1, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION));

		LevelFileHandler::saveLevelToFile(*this);
	}
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

glm::ivec2 Level::getPlayableAreaSize() const
{
	return m_size;
}

int Level::getFactionStartingResources() const
{
	return m_factionStartingResources;
}

int Level::getFactionStartingPopulationCap() const
{
	return m_factionStartingPopulationCap;
}

const std::string& Level::getName() const
{
	return m_levelName;
}

const std::vector<std::unique_ptr<Player>>& Level::getPlayers() const
{
	return m_players;
}

const GameObjectManager& Level::getGameObjectManager() const
{
	return m_gameObjectManager;
}

void Level::handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime, glm::uvec2 windowSize)
{
	switch (currentSFMLEvent.type)
	{
	case sf::Event::KeyPressed:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			m_gameObjectManager.removeAllSelectedEntities();
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
				if (camera.getRayToGroundIntersection(window, windowSize, mouseToGroundPosition))
				{
					m_translateObject.select(mouseToGroundPosition);
					if (m_translateObject.isSelected() &&
						(m_gameObjectManager.isGameObjectSelected() || m_selectedPlayer))
					{
						sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
					}
					else
					{
						const GameObject* entitySelected = m_gameObjectManager.selectGameObjectAtPosition(mouseToGroundPosition);
						if (entitySelected)
						{
							m_translateObject.setPosition(entitySelected->getPosition());
							m_plannedEntity.model = nullptr;
						}
						else
						{
							if (m_plannedEntity.model)
							{
								m_gameObjectManager.addGameObject(*m_plannedEntity.model, m_plannedEntity.position);
							}
							else
							{
								m_selectedPlayer = nullptr;

								auto player = std::find_if(m_players.begin(), m_players.end(), [&mouseToGroundPosition](const auto& player)
								{
									return player->HQ.getAABB().contains(mouseToGroundPosition);
								});
								if (player != m_players.cend())
								{
									m_translateObject.setPosition(player->get()->HQ.getPosition());
									m_selectedPlayer = (*player).get();
								}
							}
						}
					}
				}
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
			{
				m_plannedEntity.model = nullptr;
			}
		}
	}
	case sf::Event::MouseMoved:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
		{
			glm::vec3 mouseToGroundPosition = { 0.0f, 0.0f, 0.0f };
			if (camera.getRayToGroundIntersection(window, windowSize, mouseToGroundPosition))
			{
				if (m_plannedEntity.model)
				{
					glm::vec3 newPosition = Globals::convertToNodePosition(mouseToGroundPosition);
					AABB AABB(newPosition, *m_plannedEntity.model);
					if (Globals::isWithinMapBounds(AABB, m_size))
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
					case eAxisCollision::Y:
					{
						int xDifference = window.getSize().y / 2 - sf::Mouse::getPosition(window).y;

						m_translateObject.setPosition({ position.x, position.y + static_cast<float>(xDifference) * ENTITY_TRANSLATE_SPEED * deltaTime, position.z });
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

					GameObject* selectedGameObject = m_gameObjectManager.getSelectedGameObject();
					if (selectedGameObject)
					{
						glm::vec3 nodePosition = Globals::convertToNodePosition(m_translateObject.getPosition());
						selectedGameObject->setPosition(nodePosition);
						selectedGameObject->resetAABB();
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

	const std::vector<std::string>& modelNames = ModelManager::getInstance().getModelNames();
	assert(m_plannedEntity.modelNameIDSelected >= 0 && m_plannedEntity.modelNameIDSelected < modelNames.size());
	for (int i = 0; i < modelNames.size(); i++)	
	{
		if (ModelManager::getInstance().isModelLoaded(modelNames[i]) && 
			ImGui::Selectable(modelNames[i].c_str(), m_plannedEntity.modelNameIDSelected == i))
		{
			m_plannedEntity.modelNameIDSelected = i;
			m_plannedEntity.model = &ModelManager::getInstance().getModel(modelNames[i]);
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

				m_players.emplace_back(std::make_unique<Player>(factionController, PLAYER_HQ_STARTING_POSITION, PLAYER_MINERAL_STARTING_POSITION));
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
		if (m_players[i]->controller == eFactionController::Player)
		{
			ImGui::Text(std::string("Player " + std::to_string(i)).c_str());
		}
		else
		{
			ImGui::Text(std::string("AI " + std::to_string(i)).c_str());
		}

		glm::vec3 playerPosition = m_players[i]->HQ.getPosition();
		if (ImGui::InputFloat("x", &playerPosition.x, Globals::NODE_SIZE) ||
			ImGui::InputFloat("z", &playerPosition.z, Globals::NODE_SIZE))
		{
			for (auto& mineral : m_players[i]->minerals)
			{
				glm::vec3 vDifference = playerPosition - m_players[i]->HQ.getPosition();
				const glm::vec3& mineralPosition = mineral.getPosition();
				mineral.setPosition({ mineralPosition.x + vDifference.x, mineralPosition.y + vDifference.y, mineralPosition.z + vDifference.z });
				mineral.resetAABB();
			}

			m_players[i]->HQ.setPosition(playerPosition);
			m_players[i]->HQ.resetAABB();
		}
	}

	ImGui::EndChild();
}

void Level::handleSelectedEntityGUI()
{
	GameObject* selectedGameObject = m_gameObjectManager.getSelectedGameObject();
	if (selectedGameObject)
	{
		ImGui::BeginChild("Selected Entity", ImVec2(175, 275), true);
		ImGui::Text("Selected Entity");
		
		ImGui::NewLine();
		ImGui::Text("Position");
		if (ImGui::InputFloat("x", &selectedGameObject->getPosition().x, Globals::NODE_SIZE) ||
			ImGui::InputFloat("y", &selectedGameObject->getPosition().y, 1.0f) ||
			ImGui::InputFloat("z", &selectedGameObject->getPosition().z, Globals::NODE_SIZE))
		{
			selectedGameObject->resetAABB();
			m_translateObject.setPosition(selectedGameObject->getPosition());
		}

		ImGui::NewLine();
		ImGui::Text("Rotation");
		if(ImGui::InputFloat("yy", &selectedGameObject->getRotation().y, 90.0f))
		{
			if (glm::abs(selectedGameObject->getRotation().y) >= 360.0f)
			{
				selectedGameObject->setRotation({ selectedGameObject->getRotation().x, 0.0f, selectedGameObject->getRotation().z });
			}
		}

		ImGui::EndChild();
	}
}

void Level::handleLevelDetailsGUI(bool& showGUIWindow)
{
	ImGui::Begin("Level Details", &showGUIWindow, ImGuiWindowFlags_None);
	if (ImGui::InputInt("x", &m_size.x, 1) ||
		ImGui::InputInt("z", &m_size.y, 1))
	{
		m_size.x = glm::clamp(m_size.x, 0, MAX_MAP_SIZE);
		m_size.y = glm::clamp(m_size.y, 0, MAX_MAP_SIZE);
	}
	ImGui::Text("Starting Resources");
	if (ImGui::InputInt("Resources", &m_factionStartingResources, 5))
	{
		if (m_factionStartingResources < Globals::WORKER_RESOURCE_COST)
		{
			m_factionStartingResources = Globals::WORKER_RESOURCE_COST;
		}
	}
	ImGui::Text("Starting Population Cap");
	if (ImGui::InputInt("Population", &m_factionStartingPopulationCap, 1))
	{
		if (m_factionStartingPopulationCap < Globals::WORKER_POPULATION_COST)
		{
			m_factionStartingPopulationCap = Globals::WORKER_POPULATION_COST;
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
	m_gameObjectManager.render(shaderHandler);

	for (const auto& player : m_players)
	{
		player->render(shaderHandler);
	}

	if (m_plannedEntity.model)
	{
		m_plannedEntity.model->render(shaderHandler, m_plannedEntity.position);
	}

	m_translateObject.render(shaderHandler, m_gameObjectManager.isGameObjectSelected() || m_selectedPlayer);
}

void Level::renderPlayableArea(ShaderHandler& shaderHandler) const
{
	m_playableArea.render(shaderHandler, glm::vec3(0.0f),
		glm::vec3(m_size.x * Globals::NODE_SIZE, 0.0f, m_size.y * Globals::NODE_SIZE), PLAYABLE_AREA_GROUND_COLOR, PLAYABLE_AREA_OPACITY);
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	m_translateObject.renderAABB(shaderHandler, m_gameObjectManager.isGameObjectSelected() || m_selectedPlayer);
	m_gameObjectManager.renderEntityAABB(shaderHandler);

	for (const auto& player : m_players)
	{
		player->renderAABB(shaderHandler);
	}
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
			level.m_players.emplace_back(std::make_unique<Player>(factionControllerDetails.controller));
			break;
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			if (LevelFileHandler::isPlayerActive(file, factionControllerDetails.controller))
			{
				level.m_players.emplace_back(std::make_unique<Player>(factionControllerDetails.controller));
			}
			break;
		default:
			assert(false);
		}
	}

	level.m_size = LevelFileHandler::loadMapSizeFromFile(file);
	level.m_factionStartingResources = LevelFileHandler::loadFactionStartingResources(file);
	level.m_factionStartingPopulationCap = LevelFileHandler::loadFactionStartingPopulationCap(file);

	for (const auto& player : level.m_players)
	{
		file >> *player;
	}

	file >> level.m_gameObjectManager;

	return file;
}

