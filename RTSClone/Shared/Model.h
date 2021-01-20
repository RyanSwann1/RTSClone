#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>
#include <memory>
#include <vector>

enum class eFactionController;
class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& fileName, bool renderFromCentrePosition, 
		const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale);

	void render(ShaderHandler& shaderHandler, const glm::vec3& position, glm::vec3 rotation = glm::vec3()) const;
#ifdef GAME
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController, const glm::vec3& position,
		glm::vec3 rotation, bool highlight = false) const;
#endif // GAME

	const std::string modelName;
	const bool renderFromCentrePosition;
	const glm::vec3 AABBSizeFromCenter;
	const glm::vec3 scale;
	const std::vector<Mesh> meshes;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, const glm::vec3& scale,
		const std::string& fileName, std::vector<Mesh>&& meshes);

	void attachMeshesToVAO() const;
	void setModelMatrix(ShaderHandler& shaderHandler, glm::vec3 position, const glm::vec3& rotation) const;
};