#include "Level.h"
#include "FactionController.h"
#include "LevelFileHandler.h"
#include "Camera.h"
#include "imgui/imgui.h"
#include "ModelManager.h"
#include "glad.h"
#include "ShaderHandler.h"
#include <fstream>

namespace
{
	const std::array<glm::vec3, static_cast<size_t>(eFactionController::Max) + 1> FACTION_STARTING_POSITIONS =
	{
		glm::vec3(4.0f * static_cast<float>(Globals::NODE_SIZE),
		Globals::GROUND_HEIGHT,
		3.0f * static_cast<float>(Globals::NODE_SIZE)),

		glm::vec3(11.0f * static_cast<float>(Globals::NODE_SIZE), 
		Globals::GROUND_HEIGHT, 
		3.0f * static_cast<float>(Globals::NODE_SIZE)),

		glm::vec3(4.0f * static_cast<float>(Globals::NODE_SIZE),
		Globals::GROUND_HEIGHT,
		13.0f * static_cast<float>(Globals::NODE_SIZE)),

		glm::vec3(14.0f * static_cast<float>(Globals::NODE_SIZE),
		Globals::GROUND_HEIGHT,
		13.0f * static_cast<float>(Globals::NODE_SIZE)),
	};

	const int MAX_MAP_SIZE = 60 * Globals::NODE_SIZE;
	const int DEFAULT_STARTING_RESOURCES = 100;
	const int DEFAULT_STARTING_POPULATION_CAP = 5;
	const glm::ivec2 DEFAULT_MAP_SIZE = { 30, 30 };
	const float ENTITY_TRANSLATE_SPEED = 5.0f;
	const glm::vec3 PLAYABLE_AREA_GROUND_COLOR = { 1.0f, 1.0f, 0.5f };
	const float PLAYABLE_AREA_OPACITY = 0.1f;

	const glm::vec3 MAIN_BASE_QUAD_COLOR = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 MAIN_BASE_QUAD_SIZE = { Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE };
	const int MIN_FACTIONS = 2;
	const int DEFAULT_FACTIONS_COUNT = 2;

	BaseLocation* getBaseLocation(std::vector<BaseLocation>& baseLocations, const glm::vec3& position)
	{
		auto baseLocation = std::find_if(baseLocations.begin(), baseLocations.end(), [&position](const auto& baseLocation)
		{
			return baseLocation.quad.getAABB().contains(position);
		});

		if (baseLocation != baseLocations.end())
		{
			return &(*baseLocation);
		}

		return nullptr;
	}

	Mineral* getBaseLocationMineral(std::vector<BaseLocation>& baseLocations, const glm::vec3& position)
	{
		for (auto& baseLocation : baseLocations)
		{
			for (auto& mineral : baseLocation.minerals)
			{
				if (mineral.getAABB().contains(position))
				{
					return &mineral;
				}
			}
		}

		return nullptr;
	}
}

//PlannedEntity
PlannedEntity::PlannedEntity()
	: modelNameIDSelected(0),
	position(),
	model(nullptr)
{}

//BaseLocation
BaseLocation::BaseLocation(const glm::vec3& position)
	: quad(position, MAIN_BASE_QUAD_SIZE, MAIN_BASE_QUAD_COLOR),
	minerals()
{}

void BaseLocation::setPosition(const glm::vec3 & position)
{
	quad.setPosition(position);

	for (auto& mineral : minerals)
	{
		mineral.setPosition(position);
	}
}

//Level
Level::Level(const std::string& levelName)
	: m_levelName(levelName),
	m_plannedEntity(),
	m_size(DEFAULT_MAP_SIZE),
	m_playableArea({ m_size.x * Globals::NODE_SIZE, 0.0f, m_size.y * Globals::NODE_SIZE }, 
		PLAYABLE_AREA_GROUND_COLOR, PLAYABLE_AREA_OPACITY),
	m_gameObjectManager(),
	m_selectedGameObject(nullptr),
	m_selectedBaseLocation(nullptr),
	m_selectedMineral(nullptr),
	m_factionStartingResources(DEFAULT_STARTING_RESOURCES),
	m_factionStartingPopulationCap(DEFAULT_STARTING_POPULATION_CAP),
	m_factionCount(DEFAULT_FACTIONS_COUNT)
{
	if (!LevelFileHandler::loadLevelFromFile(*this))
	{
		LevelFileHandler::saveLevelToFile(*this);
	}

	m_mainBaseLocations.reserve(static_cast<size_t>(eFactionController::Max) + 1);
	m_mainBaseLocations.emplace_back(FACTION_STARTING_POSITIONS[static_cast<int>(eFactionController::Player)]);
	m_mainBaseLocations.emplace_back(FACTION_STARTING_POSITIONS[static_cast<int>(eFactionController::AI_1)]);
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

const std::string& Level::getName() const
{
	return m_levelName;
}

void Level::handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window, float deltaTime, glm::uvec2 windowSize)
{
	switch (currentSFMLEvent.type)
	{
	case sf::Event::KeyPressed:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			if (m_selectedGameObject)
			{
				m_gameObjectManager.removeGameObject(*m_selectedGameObject);
				m_selectedGameObject = nullptr;
			}
		}
		break;
	case sf::Event::MouseButtonReleased:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow) &&
			!ImGui::IsAnyItemHovered())
		{
			glm::vec3 planeIntersection(0.0f);
			if (camera.getRayToGroundIntersection(window, windowSize, planeIntersection))
			{
				if (m_selectedGameObject && 
					m_gameObjectManager.getGameObject(planeIntersection) != m_selectedGameObject)
				{
					m_selectedGameObject = nullptr;
				}
			}
			else
			{
				m_selectedGameObject = nullptr;
			}

			m_selectedBaseLocation = nullptr;
			m_selectedMineral = nullptr;
			m_selectedGameObject = nullptr;
		}
		break;
	case sf::Event::MouseButtonPressed:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow) &&
			!ImGui::IsAnyItemHovered())
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				glm::vec3 planeIntersection(0.0f);
				if (camera.getRayToGroundIntersection(window, windowSize, planeIntersection))
				{
					if (m_plannedEntity.model)
					{
						m_gameObjectManager.addGameObject(*m_plannedEntity.model, m_plannedEntity.position);
					}
					else
					{
						m_selectedGameObject = m_gameObjectManager.getGameObject(planeIntersection);
						if (!m_selectedGameObject)
						{
							m_selectedBaseLocation = getBaseLocation(m_mainBaseLocations, planeIntersection);
							if (!m_selectedBaseLocation)
							{
								m_selectedMineral = getBaseLocationMineral(m_mainBaseLocations, planeIntersection);
							}
						}
					}
				}
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
			{
				m_plannedEntity.model = nullptr;
				m_selectedGameObject = nullptr;
				m_selectedBaseLocation = nullptr;
			}
		}
	break;
	case sf::Event::MouseMoved:
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow) &&
			!ImGui::IsAnyItemHovered())
		{
			glm::vec3 planeIntersection(0.0f);
			if (camera.getRayToGroundIntersection(window, windowSize, planeIntersection))
			{
				if (m_plannedEntity.model)
				{
					glm::vec3 newPosition = Globals::convertToNodePosition(planeIntersection);
					if (Globals::isWithinMapBounds({ newPosition, *m_plannedEntity.model }, m_size))
					{
						m_plannedEntity.position = newPosition;
					}
				}
				else if (m_selectedGameObject && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					m_selectedGameObject->setPosition(Globals::convertToNodePosition(planeIntersection));
				}
				else if (m_selectedBaseLocation && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					m_selectedBaseLocation->setPosition(Globals::convertToNodePosition(planeIntersection));
				}
				else if (m_selectedMineral && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					m_selectedMineral->setPosition(Globals::convertToNodePosition(planeIntersection));
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
	ImGui::BeginChild("Faction Quantity", ImVec2(175, 40), true);
	int newFactionCount = m_factionCount;
	if (ImGui::InputInt("Faction Amount", &newFactionCount, 1, ImGuiInputTextFlags_ReadOnly))
	{
		if (newFactionCount >= MIN_FACTIONS && newFactionCount <= static_cast<int>(eFactionController::Max) + 1)
		{
			m_factionCount = newFactionCount;
		}
	};

	ImGui::EndChild();
}

void Level::handleSelectedEntityGUI()
{
	if (m_selectedGameObject)
	{
		ImGui::BeginChild("Selected Entity", ImVec2(175, 275), true);
		ImGui::Text("Selected Entity");
		
		ImGui::NewLine();
		ImGui::Text("Position");
		glm::vec3 position = m_selectedGameObject->getPosition();
		if (ImGui::InputFloat("x", &position.x, Globals::NODE_SIZE) ||
			ImGui::InputFloat("y", &position.y, 1.0f) ||
			ImGui::InputFloat("z", &position.z, Globals::NODE_SIZE))
		{
			m_selectedGameObject->setPosition(position);
		}

		ImGui::NewLine();
		ImGui::Text("Rotation");
		if(ImGui::InputFloat("yy", &m_selectedGameObject->getRotation().y, 90.0f))
		{
			if (glm::abs(m_selectedGameObject->getRotation().y) >= 360.0f)
			{
				m_selectedGameObject->setRotation({ m_selectedGameObject->getRotation().x, 0.0f, m_selectedGameObject->getRotation().z });
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
		m_playableArea.setSize({ m_size.x * Globals::NODE_SIZE, 0.0f, m_size.y * Globals::NODE_SIZE});
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

void Level::handleMainBasesGui()
{
	ImGui::BeginChild("Main Base Quantity", ImVec2(175, 40), true);
	int newMainBaseQuantity = static_cast<int>(m_mainBaseLocations.size());
	if (ImGui::InputInt("Main Bases", &newMainBaseQuantity, 1, ImGuiInputTextFlags_ReadOnly))
	{
		if (newMainBaseQuantity >= MIN_FACTIONS && newMainBaseQuantity <= static_cast<int>(eFactionController::Max) + 1)
		{
			if (newMainBaseQuantity > static_cast<int>(m_mainBaseLocations.size()))
			{
				m_mainBaseLocations.emplace_back(FACTION_STARTING_POSITIONS[newMainBaseQuantity - 1]);
			}
			else
			{
				m_mainBaseLocations.pop_back();
			}
		}
	};

	ImGui::EndChild();
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
	m_gameObjectManager.render(shaderHandler, m_selectedGameObject);

	for (const auto& baseLocation : m_mainBaseLocations)
	{
		for (const auto& mineral : baseLocation.minerals)
		{
			mineral.render(shaderHandler);
		}
	}

	if (m_plannedEntity.model)
	{
		m_plannedEntity.model->render(shaderHandler, m_plannedEntity.position);
	}

	ModelManager::getInstance().getModel(TERRAIN_MODEL_NAME).render(shaderHandler, Globals::TERRAIN_POSITION);
}

void Level::renderDebug(ShaderHandler& shaderHandler) const
{
	m_playableArea.render(shaderHandler);
	
	for (const auto& baseLocation : m_mainBaseLocations)
	{
		baseLocation.quad.render(shaderHandler);
	}
}

#ifdef RENDER_AABB
void Level::renderAABB(ShaderHandler& shaderHandler)
{
	m_gameObjectManager.renderGameObjectAABB(shaderHandler);
}
#endif // RENDER_AABB

const std::ifstream& operator>>(std::ifstream& file, Level& level)
{
	assert(file.is_open());

	level.m_size = LevelFileHandler::loadMapSizeFromFile(file);
	level.m_factionStartingResources = LevelFileHandler::loadFactionStartingResources(file);
	level.m_factionStartingPopulationCap = LevelFileHandler::loadFactionStartingPopulationCap(file);

	file >> level.m_gameObjectManager;

	return file;
}

std::ostream& operator<<(std::ostream& file, const Level& level)
{
	file << Globals::TEXT_HEADER_FACTION_COUNT << "\n";
	file << level.m_factionCount << "\n";
	
	for(int i = 0; i < static_cast<int>(level.m_mainBaseLocations.size()); ++i)
	{
		file << Globals::TEXT_HEADER_MAIN_BASES[i] << "\n";
		const glm::vec3& basePosition = level.m_mainBaseLocations[i].quad.getPosition();
		file << basePosition.x << " " << basePosition.y << " " << basePosition.z << "\n";
		file << static_cast<int>(level.m_mainBaseLocations[i].minerals.size()) << "\n";
		for (const auto& mineral : level.m_mainBaseLocations[i].minerals)
		{
			file << mineral.getPosition().x << " " << mineral.getPosition().y << " " << mineral.getPosition().z << "\n";
		}
	}

	file << Globals::TEXT_HEADER_FACTION_STARTING_POPULATION << "\n";
	file << level.m_factionStartingPopulationCap << "\n";

	file << Globals::TEXT_HEADER_FACTION_STARTING_RESOURCE << "\n";
	file << level.m_factionStartingResources << "\n";

	file << Globals::TEXT_HEADER_MAP_SIZE << "\n";
	file << level.m_size.x << " " << level.m_size.y << "\n";
	
	file << level.m_gameObjectManager;

	return file;
}