#include "Faction.h"
#include "Globals.h"
#include "glad.h"
#include "Unit.h"
#include "Headquarters.h"
#include "Camera.h"
#include "Map.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include <assert.h>
#include <array>
#include <algorithm>

namespace
{
    constexpr int STARTING_RESOURCES = 50;
    constexpr int WORKER_RESOURCE_COST = 50;
    constexpr int SUPPLY_DEPOT_COST = 50;
    constexpr int UNIT_RESOURCE_COST = 100;
    constexpr int STARTING_POPULATION = 5;
    constexpr int MAX_POPULATION = 20;
    constexpr int WORKER_POPULATION_COST = 1;
    constexpr int UNIT_POPULATION_COST = 2;
    constexpr int POPULATION_INCREMENT = 5;

    std::array<glm::ivec2, 6> getSelectionBoxQuadCoords(const glm::ivec2& startingPosition, const glm::ivec2& size)
    {
        return
        {
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y))
        };
    };
};

//SelectionBox
SelectionBox::SelectionBox()
    : AABB(),
    active(false),
    mouseToGroundPosition(),
    startingPositionScreenPosition(),
    startingPositionWorldPosition(),
    vaoID(Globals::INVALID_OPENGL_ID),
    vboID(Globals::INVALID_OPENGL_ID)
{
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);
}

SelectionBox::~SelectionBox()
{
    assert(vaoID != Globals::INVALID_OPENGL_ID);
    glDeleteVertexArrays(1, &vaoID);

    assert(vboID != Globals::INVALID_OPENGL_ID);
    glDeleteBuffers(1, &vboID);
}

void SelectionBox::setStartingPosition(const sf::Window& window, const glm::vec3& position)
{
    startingPositionWorldPosition = position;
    startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
    active = true;
}

void SelectionBox::setSize(const glm::vec3& position)
{
    mouseToGroundPosition = position;
    AABB.reset(startingPositionWorldPosition, mouseToGroundPosition - startingPositionWorldPosition);
}

void SelectionBox::reset()
{
    active = false;
    AABB.reset();
}

void SelectionBox::render(const sf::Window& window) const
{
    if (active)
    {
        glm::vec2 endingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        std::array<glm::ivec2, 6> quadCoords = getSelectionBoxQuadCoords(startingPositionScreenPosition,
            endingPosition - startingPositionScreenPosition);

        glBindVertexArray(vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::ivec2), quadCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(glm::ivec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

//Faction
Faction::Faction(const ModelManager& modelManager, Map& map)
    : m_currentResourceAmount(STARTING_RESOURCES),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(STARTING_POPULATION),
    m_selectionBox(),
    m_HQ(Globals::convertToNodePosition({ 35.0f, Globals::GROUND_HEIGHT, 15.f }), modelManager.getModel(eModelName::HQ), map),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_previousMouseToGroundPosition()
{
    std::cout << "Resources: " <<  m_currentResourceAmount << "\n";
    std::cout << "Current Population: " << m_currentPopulationAmount << "\n";
}

void Faction::addResources(Worker & worker)
{
    m_currentResourceAmount += worker.extractResources();
    std::cout << "Resources: " << m_currentResourceAmount << "\n";
}

void Faction::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Map& map, 
    const ModelManager& modelManager, const std::vector<Mineral>& minerals, float deltaTime)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            bool selectAllUnits = false;
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            if (mouseToGroundPosition != m_previousMouseToGroundPosition)
            {
                m_previousMouseToGroundPosition = mouseToGroundPosition;
                selectAllUnits = false;
            }
            else
            {
                selectAllUnits = true;
            }

            selectUnit<Unit>(m_units, mouseToGroundPosition, selectAllUnits);
            selectUnit<Worker>(m_workers, mouseToGroundPosition, selectAllUnits);

            m_selectionBox.setStartingPosition(window, mouseToGroundPosition);
            m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            if (m_HQ.isSelected())
            {
                m_HQ.setWaypointPosition(mouseToGroundPosition);
            }
            else
            {
                if (isOneUnitSelected())
                {
                    moveSingularSelectedUnit(mouseToGroundPosition, map, minerals);
                }
                else
                {
                    moveMultipleSelectedUnits(mouseToGroundPosition, map, minerals);
                }
            }
        }
        break;
    case sf::Event::MouseButtonReleased:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_selectionBox.reset();
        }
        break;
    case sf::Event::MouseMoved:
        if (m_selectionBox.active)
        {
            m_selectionBox.setSize(camera.getMouseToGroundPosition(window));

            for (auto& unit : m_units)
            {
                unit.setSelected(m_selectionBox.AABB.contains(unit.getAABB()));
            }

            for (auto& worker : m_workers)
            {
                worker.setSelected(m_selectionBox.AABB.contains(worker.getAABB()));
            }
        }
        break;
    case sf::Event::KeyPressed:
        switch (currentSFMLEvent.key.code)
        {
        case sf::Keyboard::Num1:
            if (m_HQ.isSelected() && isEntityAffordable(eEntityType::Worker) && 
                !isExceedPopulationLimit(eEntityType::Worker))
            {
                spawnUnit<Worker>(m_HQ.getUnitSpawnPosition(), modelManager.getModel(eModelName::Worker), map, m_workers, 
                    eEntityType::Worker);
                
                reduceResources(eEntityType::Worker);
                increaseCurrentPopulationAmount(WORKER_POPULATION_COST, eEntityType::Worker);
            }
            break;
        case sf::Keyboard::Num2:
            if (m_HQ.isSelected() && isEntityAffordable(eEntityType::Worker) &&
                !isExceedPopulationLimit(eEntityType::Unit))
            {
                spawnUnit<Unit>(m_HQ.getUnitSpawnPosition(), modelManager.getModel(eModelName::Unit), map, m_units, eEntityType::Unit);
                
                reduceResources(eEntityType::Worker);
                increaseCurrentPopulationAmount(UNIT_POPULATION_COST, eEntityType::Unit);
            }
            break;
        case sf::Keyboard::B:
        {
            glm::vec3 position = Globals::convertToNodePosition(camera.getMouseToGroundPosition(window));
            if (m_currentPopulationLimit + POPULATION_INCREMENT < MAX_POPULATION &&
                isEntityAffordable(eEntityType::SupplyDepot) &&
                PathFinding::getInstance().isPositionAvailable(position, map, m_units, m_workers))
            {
                m_supplyDepots.emplace_back(position, modelManager.getModel(eModelName::SupplyDepot), map);   
                
                reduceResources(eEntityType::SupplyDepot);
                increasePopulationLimit();
            }  

            revalidateExistingUnitPaths(map);
        }
            break;
        }
        break;
    }
}

void Faction::update(float deltaTime, const ModelManager& modelManager, const Map& map)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime, modelManager);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, modelManager, m_HQ, map, *this);
    }

    handleCollisions<Unit>(m_units, map);
    handleCollisions<Worker>(m_workers, map);
}

void Faction::render(ShaderHandler& shaderHandler, const ModelManager& modelManager) const
{
    m_HQ.render(shaderHandler, modelManager.getModel(m_HQ.getModelName()), modelManager.getModel(eModelName::Waypoint));

    for (auto& unit : m_units)
    {
        unit.render(shaderHandler, modelManager.getModel(unit.getModelName()));
    }

    for (auto& worker : m_workers)
    {
        worker.render(shaderHandler, modelManager.getModel(worker.getModelName()));
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.render(shaderHandler, modelManager.getModel(supplyDepot.getModelName()));
    }
}

void Faction::renderSelectionBox(const sf::Window& window) const
{
    m_selectionBox.render(window);
}

#ifdef RENDER_PATHING
void Faction::renderPathing(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_units)
    {
        unit.renderPathMesh(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.renderPathMesh(shaderHandler);
    }
}
#endif // RENDER_PATHING

#ifdef RENDER_AABB
void Faction::renderAABB(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_units)
    {
        unit.renderAABB(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.renderAABB(shaderHandler);
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.renderAABB(shaderHandler);
    }

    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

bool Faction::isExceedPopulationLimit(eEntityType entityType) const
{
    switch (entityType)
    {
    case eEntityType::Unit:
        return m_currentPopulationAmount + UNIT_POPULATION_COST > m_currentPopulationLimit;
    case eEntityType::Worker:
        return m_currentPopulationAmount + WORKER_POPULATION_COST > m_currentPopulationLimit;
    default:
        assert(false);
        return true;
    }
}

bool Faction::isEntityAffordable(eEntityType entityType) const
{
    switch (entityType)
    {
    case eEntityType::Worker:
        return m_currentResourceAmount - WORKER_RESOURCE_COST >= 0;
    case eEntityType::Unit:
        return  m_currentResourceAmount - UNIT_RESOURCE_COST >= 0;
    case eEntityType::SupplyDepot:
        return m_currentResourceAmount - SUPPLY_DEPOT_COST >= 0;
    default:
        assert(false);
        return false;
    }
}

bool Faction::isOneUnitSelected() const
{
    int unitSelectedCount = 0;

    for (const auto& unit : m_units)
    {
        if (unit.isSelected())
        {
            ++unitSelectedCount;
            if (unitSelectedCount > 1)
            {
                return false;
            }
        }
    }

    for (const auto& worker : m_workers)
    {
        if (worker.isSelected())
        {
            ++unitSelectedCount;
            if (unitSelectedCount > 1)
            {
                return false;
            }
        }
    }

    return unitSelectedCount == 1;
}

void Faction::moveSingularSelectedUnit(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals)
{
    assert(isOneUnitSelected());

    auto selectedUnit = std::find_if(m_units.begin(), m_units.end(), [](const auto& unit) {
        return unit.isSelected() == true;
    });
    if (selectedUnit != m_units.end())
    {
        selectedUnit->moveTo(Globals::convertToNodePosition(destinationPosition), map, m_units,
            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map, m_units, *selectedUnit); });
    }
    else
    {
        auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [](const auto& worker) {
            return worker.isSelected() == true;
        });
        assert(selectedWorker != m_workers.end());

        selectedWorker->moveTo(destinationPosition, map, minerals);
    }
}

void Faction::moveMultipleSelectedUnits(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals)
{
    static std::vector<Unit*> selectedUnits;
 
    for (auto& unit : m_units)
    {
        if (unit.isSelected())
        {
            selectedUnits.push_back(&unit);
        }
    }

    for (auto& worker : m_workers)
    {
        if (worker.isSelected())
        {
            selectedUnits.push_back(&worker);
        }
    }   
    
    if (!selectedUnits.empty())
    {
        assert(!isOneUnitSelected());
        AABB selectionBoxAABB({ selectedUnits.begin(), selectedUnits.end() });
        if (selectionBoxAABB.contains(destinationPosition))
        {
            std::vector<glm::vec3> unitFormationPositions = PathFinding::getInstance().getFormationPositions(destinationPosition, 
                selectedUnits, map);

            if (unitFormationPositions.size() == selectedUnits.size())
            {
                for (int i = 0; i < unitFormationPositions.size(); ++i)
                {
                    selectedUnits[i]->moveTo(unitFormationPositions[i], map, m_units,
                        [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
                }
            }
        }
        else
        {
            std::sort(selectedUnits.begin(), selectedUnits.end(), [](const auto& unitA, const auto& unitB)
            {
                return glm::all(glm::lessThan(unitA->getPosition(), unitB->getPosition()));
            });

            glm::vec3 total(0.0f, 0.0f, 0.0f);
            for (const auto& selectedUnit : selectedUnits)
            {
                total += selectedUnit->getPosition();
            }

            glm::vec3 averagePosition = { total.x / selectedUnits.size(), total.y / selectedUnits.size(), total.z / selectedUnits.size() };

            for (auto& selectedUnit : selectedUnits)
            {
                switch (selectedUnit->getType())
                {
                case eEntityType::Unit:
                    selectedUnit->moveTo(Globals::convertToNodePosition(destinationPosition - (averagePosition - selectedUnit->getPosition())), map, m_units,
                        [&](const glm::ivec2& position)
                    { return getAllAdjacentPositions(position, map, m_units, *selectedUnit, selectedUnits); });
                    break;
                case eEntityType::Worker:
                    static_cast<Worker*>(selectedUnit)->moveTo(destinationPosition - (averagePosition - selectedUnit->getPosition()), map);
                    break;
                default:
                    assert(false);
                }
            }
        }
    }

    selectedUnits.clear();
}

void Faction::revalidateExistingUnitPaths(const Map& map)
{
    for (auto& unit : m_units)
    {
        if (!unit.isPathEmpty())
        {
            glm::vec3 destination = unit.getDestination();
            unit.moveTo(destination, map, m_units, [&](const glm::ivec2& position) 
                { return getAllAdjacentPositions(position, map, m_units, unit); });
        }
    }

    for (auto& worker : m_workers)
    {
        if (!worker.isPathEmpty())
        {
            glm::vec3 destination = worker.getDestination();
            worker.moveTo(destination, map, worker.getCurrentState());
        }
    }
}

void Faction::reduceResources(eEntityType addedEntityType)
{
    assert(isEntityAffordable(addedEntityType));
    switch (addedEntityType)
    {
    case eEntityType::Unit:
        m_currentResourceAmount -= UNIT_RESOURCE_COST;
        break;
    case eEntityType::Worker:
        m_currentResourceAmount -= WORKER_RESOURCE_COST;
        break;
    case eEntityType::SupplyDepot:
        m_currentResourceAmount -= SUPPLY_DEPOT_COST;
        break;
    }

    std::cout << "Resources: " <<  m_currentResourceAmount << "\n";
}

void Faction::increaseCurrentPopulationAmount(int amount, eEntityType entityType)
{
    assert(!isExceedPopulationLimit(entityType));
    m_currentPopulationAmount += amount;

    std::cout << "Population: " << m_currentPopulationAmount << "\n";
}

void Faction::increasePopulationLimit()
{
    assert(m_currentPopulationLimit + POPULATION_INCREMENT <= MAX_POPULATION);
    m_currentPopulationLimit += POPULATION_INCREMENT;

    std::cout << "Population Limit: " << m_currentPopulationLimit << "\n";
}