#include "Mesh.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "glad.h"

//Mesh
Mesh::Mesh()
	: vertices(),
	indices(),
	textures(),
	vaoID(Globals::INVALID_OPENGL_ID),
	vboID(Globals::INVALID_OPENGL_ID),
	indiciesID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &indiciesID);

	glBindVertexArray(vaoID);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiciesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	//positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	//normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normals));
	//textCoords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, textCoords));
}

Mesh::~Mesh()
{
	assert(vaoID != Globals::INVALID_OPENGL_ID);
	glDeleteVertexArrays(1, &vaoID);

	assert(vboID != Globals::INVALID_OPENGL_ID);
	glDeleteBuffers(1, &vboID);

	assert(indiciesID != Globals::INVALID_OPENGL_ID);
	glDeleteBuffers(1, &indiciesID);
}

void Mesh::render(ShaderHandler& shaderHandler) const
{
	unsigned int diffuseNumb = 1;
	unsigned int specularNumb = 1;
	for (unsigned int i = 0; i < static_cast<unsigned int>(textures.size()); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
		{
			number = std::to_string(diffuseNumb++);
		}
		else if (name == "texture_specular")
		{
			number = std::to_string(specularNumb++);
		}

		shaderHandler.setUniform1f(eShaderType::Default, name + number, static_cast<float>(i));
		glBindTexture(GL_TEXTURE_2D, textures[i].ID);
	}

	glActiveTexture(GL_TEXTURE0);

	// draw mesh
	glBindVertexArray(vaoID);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

//Texture
Texture::Texture()
	: ID(),
	type()
{}

//Vertex
Vertex::Vertex()
	: positions(),
	normals(),
	textCoords()
{}