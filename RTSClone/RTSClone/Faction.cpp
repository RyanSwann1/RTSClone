#include "Faction.h"

Faction::Faction(const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition)
    : m_currentResourceAmount(Globals::STARTING_RESOURCES),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(Globals::STARTING_POPULATION),
    m_minerals(),
    m_allEntities(),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_barracks(),
    m_HQ(Globals::convertToNodePosition(hqStartingPosition), eModelName::HQ, eEntityType::HQ)
{
    m_allEntities.push_back(&m_HQ);

    m_minerals.reserve(Globals::MAX_MINERALS_PER_FACTION);
    glm::vec3 startingPosition = mineralsStartingPosition;
    for (int i = 0; i < Globals::MAX_MINERALS_PER_FACTION; ++i)
    {
        glm::vec3 position = mineralsStartingPosition;
        position.z += Globals::NODE_SIZE * i;
        m_minerals.emplace_back(Globals::convertToNodePosition(position));
    }
}

int Faction::getEntityIDAtPosition(const glm::vec3& position) const
{
    auto cIter = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });

    if (cIter != m_allEntities.cend())
    {
        return (*cIter)->getID();
    }
    else
    {
        return Globals::INVALID_ENTITY_ID;
    }
}

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

    for (const auto& unit : m_units)
    {
        unit.render(shaderHandler);
    }

    for (const auto& worker : m_workers)
    {
        worker.render(shaderHandler);
    }

    for (const auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.render(shaderHandler);
    }

    for (const auto& barracks : m_barracks)
    {
        barracks.render(shaderHandler);
    }

    for (const auto& minerals : m_minerals)
    {
        minerals.render(shaderHandler);
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

    for (auto& minerals : m_minerals)
    {
        minerals.renderAABB(shaderHandler);
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
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            m_supplyDepots.emplace_back(spawnPosition);
            addedBuilding = &m_supplyDepots.back();
            increasePopulationLimit();
            break;
        case eEntityType::Barracks:
            m_barracks.emplace_back(spawnPosition, eModelName::Barracks, eEntityType::Barracks);
            addedBuilding = &m_barracks.back();
            break;
        default:
            assert(false);
        }

        reduceResources(entityType);
        revalidateExistingUnitPaths(map);

        assert(addedBuilding);
        m_allEntities.push_back(addedBuilding);
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