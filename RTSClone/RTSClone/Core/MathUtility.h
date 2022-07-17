#pragma once

#include "glm/glm.hpp"
#include <functional>
#include <numeric>
#include <vector>

namespace MathUtility
{
	//if (glm::distance(position, harvest_location.position) < distance)
	//{
	//	closest_harvest_location = &harvest_location;
	//}

	//template <typename Container, typename Type>
	//const Type* Closest(const glm::vec3& position, const Container& container, 
	//	const std::function<bool(const glm::vec3& t1, const glm::vec3& t2)>& comps)
	//{
	//	const Type* closest{ nullptr }; 
	//	float distance{ std::numeric_limits<float>::max() };
	//	for (const auto& c : container)
	//	{
	//		float new_distance{comp(position, c.)}
	//		if(comp())
	//	}
	//}
}