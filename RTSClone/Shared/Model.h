#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include "ModelName.h"
#include <string>
#include <memory>

#ifdef LEVEL_EDITOR
struct GameObject;
#endif // LEVEL_EDITOR
#ifdef GAME
class Entity;
#endif // GAME
class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& fileName, bool renderFromCentrePosition, 
		const glm::vec3& AABBSizeFromCenter, eModelName modelName, const glm::vec3& scale);

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler, const glm::vec3& position) const;

#ifdef GAME
	void render(ShaderHandler& shaderHandler, const Entity& entity) const;
#endif // GAME

#ifdef LEVEL_EDITOR
	void render(ShaderHandler& shaderHandler, const GameObject& gameObject) const;
#endif // LEVEL_EDITOR

	const eModelName modelName;
	const bool renderFromCentrePosition;
	const glm::vec3 AABBSizeFromCenter;
	const glm::vec3 scale;
	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, eModelName modelName, const glm::vec3& scale);
};