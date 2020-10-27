#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>
#include <memory>
#include <vector>

class Entity;
class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& fileName, bool renderFromCentrePosition, 
		const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale);

	void render(ShaderHandler& shaderHandler, const glm::vec3& position) const;
	void render(ShaderHandler& shaderHandler, const Entity& entity) const;

	const std::string modelName;
	const bool renderFromCentrePosition;
	const glm::vec3 AABBSizeFromCenter;
	const glm::vec3 scale;
	const std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, const glm::vec3& scale,
		const std::string& fileName, std::vector<Mesh>&& meshes);

	void render(ShaderHandler& shaderHandler, glm::vec3 entityPosition, bool entitySelected, 
		glm::vec3 rotation = glm::vec3()) const;
	void attachMeshesToVAO() const;
};