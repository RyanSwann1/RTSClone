#pragma once

#include "Entities/Unit.h"
#include "Entities/Barracks.h"
#include "Entities/Worker.h"
#include "Entities/SupplyDepot.h"
#include "Entities/Turret.h"
#include "Entities/Laboratory.h"
#include "Entities/Headquarters.h"
#include "Core/ConstSafePTR.h"
#include <vector>

class Entity;
struct FactionEntities
{
	std::vector<ConstSafePTR<Entity>> all;
	std::vector<Unit> units;
	std::vector<Worker> workers;
	std::vector<SupplyDepot> supply_depots;
	std::vector<Barracks> barracks;
	std::vector<Turret> turrets;
	std::vector<Headquarters> headquarters;
	std::vector<Laboratory> laboratories;
};