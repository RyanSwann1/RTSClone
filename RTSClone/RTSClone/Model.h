#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include "ModelName.h"
#include <string>
#include <memory>

class Entity;
class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& filePath, bool renderFromCentrePosition, 
		const glm::vec3& AABBSizeFromCenter, eModelName modelName, const glm::vec3& scale);

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler, const glm::vec3& position) const;
	void render(ShaderHandler& shaderHandler, const Entity& entity) const;

	const eModelName modelName;
	const bool renderFromCentrePosition;
	const glm::vec3 AABBSizeFromCenter;
	const glm::vec3 scale;
	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, eModelName modelName, const glm::vec3& scale);
};