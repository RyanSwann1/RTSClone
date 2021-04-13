#pragma once

#include "glm/glm.hpp"
#include <functional>
#include <vector>

struct Base;
class Worker;
struct AIOccupiedBase
{
	AIOccupiedBase(const Base& base);

	void addWorker(const Worker& worker);
	void removeWorker(const Worker& worker);

	std::reference_wrapper<const Base> base;
	std::vector<std::reference_wrapper<const Worker>> workers;
};

class Headquarters;
class BaseHandler;
class AIOccupiedBases
{
public:
	AIOccupiedBases(const BaseHandler& baseHandler);
	AIOccupiedBases(const AIOccupiedBases&) = delete;
	AIOccupiedBases& operator=(const AIOccupiedBases&) = delete;
	AIOccupiedBases(AIOccupiedBases&&) = delete;
	AIOccupiedBases& operator=(AIOccupiedBases&&) = delete;

	const std::vector<AIOccupiedBase>& getSortedBases(const glm::vec3& position);
	void addBase(const Base& base);
	void removeBase(const Base& base);

	void addWorker(const Worker& worker, const Headquarters& headquarters);
	void addWorker(const Worker& worker, const Base& base);
	void removeWorker(const Worker& worker);

private:
	std::vector<AIOccupiedBase> m_bases;
};