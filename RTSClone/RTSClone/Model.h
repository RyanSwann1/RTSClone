#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>
#include <memory>

enum class eModelName
{
	Unit = 0,
	HQ,
	Mineral,
	Waypoint,
	Harvester,
	Max = Harvester
};

class Entity;
class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& filePath, bool renderFromCentrePosition, 
		const glm::vec3& sizeFromCentre, eModelName modelName);

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler, const glm::vec3& position) const;
	void render(ShaderHandler& shaderHandler, const Entity& entity) const;

	const eModelName modelName;
	const bool renderFromCentrePosition;
	const glm::vec3 sizeFromCentre;
	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, eModelName modelName);
};