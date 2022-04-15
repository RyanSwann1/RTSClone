#include "Graphics/Mesh.h"
#include "Core/Globals.h"
#include "glad/glad.h"
#include "Graphics/ShaderHandler.h"

namespace
{
	const float HIGHLIGHTED_MESH_AMPLIFIER = 1.75f;
}

//Vertex
Vertex::Vertex(const glm::vec3& position)
	: position(position),
	normal()
{}

Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal)
	: position(position),
	normal(normal)
{}

//Material
Material::Material()
	: diffuse(),
	name()
{}

Material::Material(const glm::vec3& diffuse, const std::string& name)
	: diffuse(diffuse),
	name(name)
{}

Mesh::Mesh()
	: m_VAO(),
	m_VBO(GL_ARRAY_BUFFER),
	m_indices(GL_ELEMENT_ARRAY_BUFFER),
	vertices(),
	indices(),
	material()
{}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, const Material& material)
	: m_VAO(),
	m_VBO(GL_ARRAY_BUFFER),
	m_indices(GL_ELEMENT_ARRAY_BUFFER),
	vertices(std::move(vertices)),
	indices(std::move(indices)),
	material(material)
{}

void Mesh::attachToVAO() const
{
	m_VAO.bind();
	assert(!vertices.empty());
	m_VBO.bind();
	glBufferData(GL_ARRAY_BUFFER, 
		static_cast<GLsizei>(vertices.size() * sizeof(Vertex)), 
		vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, glm::vec3::length(), GL_FLOAT, GL_FALSE, 
		static_cast<GLsizei>(sizeof(Vertex)), 
		reinterpret_cast<const void*>(offsetof(Vertex, position)));
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, glm::vec3::length(), GL_FLOAT, GL_FALSE,
		static_cast<GLsizei>(sizeof(Vertex)),
		reinterpret_cast<const void*>(offsetof(Vertex, normal)));

	assert(!indices.empty());
	m_indices.bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
		static_cast<GLsizei>(indices.size() * sizeof(unsigned int)), 
		indices.data(), GL_STATIC_DRAW);
}

void Mesh::renderDebugMesh(ShaderHandler& shaderHandler) const
{
	m_VAO.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::render(ShaderHandler& shaderHandler, const glm::vec3& additionalColor, float opacity) const
{
	assert(!indices.empty());

	shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	shaderHandler.setUniformVec3(eShaderType::Default, "uAdditionalColour", additionalColor);
	shaderHandler.setUniform1f(eShaderType::Default, "uOpacity", opacity);

	m_VAO.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::render(ShaderHandler& shaderHandler, bool highlight) const
{
	assert(!indices.empty());

	shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	shaderHandler.setUniformVec3(eShaderType::Default, "uAdditionalColour", glm::vec3(1.0f));
	if (highlight)
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", HIGHLIGHTED_MESH_AMPLIFIER);
	}
	else
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
	}

	m_VAO.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool highlight) const
{
	assert(!indices.empty());

	shaderHandler.setUniformVec3(eShaderType::Default, "uAdditionalColour", glm::vec3(1.0f));
	if (material.name == Globals::FACTION_MATERIAL_NAME_ID)
	{
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", 
			Globals::FACTION_COLORS[static_cast<int>(owningFactionController)]);
	}
	else
	{
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	}
	
	if (highlight)
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", HIGHLIGHTED_MESH_AMPLIFIER);
	}
	else
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
	}

	m_VAO.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}