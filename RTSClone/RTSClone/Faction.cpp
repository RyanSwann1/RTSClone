#include "Faction.h"

Faction::Faction(Map& map)
    : m_currentResourceAmount(Globals::STARTING_RESOURCES),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(Globals::STARTING_POPULATION),
    m_HQ(Globals::convertToNodePosition({ 35.0f, Globals::GROUND_HEIGHT, 15.f }), map, eModelName::HQ),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_barracks()
{}

void Faction::addResources(Worker& worker)
{
    m_currentResourceAmount += worker.extractResources();
    std::cout << "Resources: " << m_currentResourceAmount << "\n";
}

void Faction::update(float deltaTime, const Map& map)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, m_HQ, map, *this);
    }

    handleCollisions<Unit>(m_units, map);
    handleCollisions<Worker>(m_workers, map);
}

void Faction::render(ShaderHandler& shaderHandler) const
{
    m_HQ.render(shaderHandler);

    for (auto& unit : m_units)
    {
        unit.render(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.render(shaderHandler);
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.render(shaderHandler);
    }

    for (auto& barracks : m_barracks)
    {
        barracks.render(shaderHandler);
    }
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

    for (auto& barracks : m_barracks)
    {
        barracks.renderAABB(shaderHandler);
    }

    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

bool Faction::isExceedPopulationLimit(eEntityType entityType) const
{
    switch (entityType)
    {
    case eEntityType::Unit:
        return m_currentPopulationAmount + Globals::UNIT_POPULATION_COST > m_currentPopulationLimit;
    case eEntityType::Worker:
        return m_currentPopulationAmount + Globals::WORKER_POPULATION_COST > m_currentPopulationLimit;
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
        return m_currentResourceAmount - Globals::WORKER_RESOURCE_COST >= 0;
    case eEntityType::Unit:
        return  m_currentResourceAmount - Globals::UNIT_RESOURCE_COST >= 0;
    case eEntityType::SupplyDepot:
        return m_currentResourceAmount - Globals::SUPPLY_DEPOT_RESOURCE_COST >= 0;
    case eEntityType::Barracks:
        return m_currentResourceAmount - Globals::BARRACKS_RESOURCE_COST >= 0;
    default:
        assert(false);
        return false;
    }
}

const Entity* Faction::addBuilding(Worker& worker, Map& map, glm::vec3 spawnPosition, eEntityType entityType)
{
    if (m_currentPopulationLimit + Globals::POPULATION_INCREMENT < Globals::MAX_POPULATION &&
        isEntityAffordable(entityType) &&
        PathFinding::getInstance().isPositionAvailable(spawnPosition, map, m_units, m_workers, worker))
    {
        const Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            m_supplyDepots.emplace_back(spawnPosition, map);
            addedBuilding = &m_supplyDepots.back();
            increasePopulationLimit();
            break;
        case eEntityType::Barracks:
            m_barracks.emplace_back(spawnPosition, map, eModelName::Barracks);
            addedBuilding = &m_barracks.back();
            break;
        default:
            assert(false);
        }

        reduceResources(entityType);
        revalidateExistingUnitPaths(map);

        assert(addedBuilding);
        return addedBuilding;
    }

    return nullptr;
}

void Faction::reduceResources(eEntityType addedEntityType)
{
    assert(isEntityAffordable(addedEntityType));
    switch (addedEntityType)
    {
    case eEntityType::Unit:
        m_currentResourceAmount -= Globals::UNIT_RESOURCE_COST;
        break;
    case eEntityType::Worker:
        m_currentResourceAmount -= Globals::WORKER_RESOURCE_COST;
        break;
    case eEntityType::SupplyDepot:
        m_currentResourceAmount -= Globals::SUPPLY_DEPOT_RESOURCE_COST;
        break;
    case eEntityType::Barracks:
        m_currentResourceAmount -= Globals::BARRACKS_RESOURCE_COST;
        break;
    }

    std::cout << "Resources: " << m_currentResourceAmount << "\n";
}

void Faction::increaseCurrentPopulationAmount(eEntityType entityType)
{
    assert(!isExceedPopulationLimit(entityType));
    switch (entityType)
    {
    case eEntityType::Unit:
        m_currentPopulationAmount += Globals::UNIT_POPULATION_COST;
        break;
    case eEntityType::Worker:
        m_currentPopulationAmount += Globals::WORKER_POPULATION_COST;
        break;
    default:
        assert(false);
    }

    std::cout << "Population: " << m_currentPopulationAmount << "\n";
}

void Faction::increasePopulationLimit()
{
    assert(m_currentPopulationLimit + Globals::POPULATION_INCREMENT <= Globals::MAX_POPULATION);
    m_currentPopulationLimit += Globals::POPULATION_INCREMENT;

    std::cout << "Population Limit: " << m_currentPopulationLimit << "\n";
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

void Faction::instructWorkerToBuild(eEntityType entityType, const glm::vec3& mouseToGroundPosition, Map& map)
{
    if (!Globals::isPositionInMapBounds(mouseToGroundPosition))
    {
        return;
    }

    switch (entityType)
    {
    case eEntityType::Barracks:
    case eEntityType::SupplyDepot:
    {
        auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [](const auto& worker)
        {
            return worker.isSelected();
        });
        if (selectedWorker != m_workers.end())
        {
            glm::vec3 buildPosition = Globals::convertToNodePosition(mouseToGroundPosition);
            selectedWorker->build([this, &map, buildPosition, entityType](Worker& worker)
            { return addBuilding(worker, map, buildPosition, entityType); }, buildPosition, map);
        }
    }
    break;
    default:
        assert(false);
    }
}