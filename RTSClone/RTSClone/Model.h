#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>
#include <memory>

class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& filePath, bool renderFromCentrePosition, const glm::vec3& sizeFromCentre);

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler, bool selected) const;

	const bool renderFromCentrePosition;
	const glm::vec3 sizeFromCentre;
	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre);
};