#include "Movement.h"

bool Movement::IsMovableAfterAddingDestination(const bool add_to_destinations, const glm::vec3& position)
{
	if (add_to_destinations && !path.empty())
	{
		destinations.push(position);
		return false;
	}

	return true;
}
