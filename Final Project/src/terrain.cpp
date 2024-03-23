#include "terrain.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <vector>
#include <framework/shader.h>



Terrain::Terrain(void)
    
{
}

void Terrain::renderTerrain(glm::mat4 view, glm::vec3 cameraPosition) {
	int SIZE = 200;
	const int VERTEX_COUNT = 128;
	const int count = VERTEX_COUNT * VERTEX_COUNT;
	float vertices[count * 3];
	float normals[count * 3];
	float textureCoords[count * 2];
	//int indices[6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1)];


	int vertexPointer = 0;
	std::vector<terrainVertex> vertices2;
	std::vector<glm::uvec3> triangles;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			vertices[vertexPointer * 3] = (float)j / ((float)VERTEX_COUNT - 1) * SIZE;
			vertices[vertexPointer * 3 + 1] = 0;
			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * SIZE;
			
			normals[vertexPointer * 3] = 0;
			normals[vertexPointer * 3 + 1] = 1;
			normals[vertexPointer * 3 + 2] = 0;
			textureCoords[vertexPointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
			textureCoords[vertexPointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);
			vertexPointer++;

			terrainVertex vertex{
						.position = {vertices[vertexPointer * 3], vertices[vertexPointer * 3 + 1], vertices[vertexPointer * 3 + 2]},
						.normal = {normals[vertexPointer * 3], normals[vertexPointer * 3 + 1], normals[vertexPointer * 3 + 2]},
						.texCoord = {textureCoords[vertexPointer * 2], textureCoords[vertexPointer * 2 + 1]}
			};

			vertices2.push_back(vertex);
		}
	}


	int pointer = 0;
	for (int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
		for (int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
			int topLeft = (gz * VERTEX_COUNT) + gx;
			int topRight = topLeft + 1;
			int bottomLeft = ((gz + 1) * VERTEX_COUNT) + gx;
			int bottomRight = bottomLeft + 1;
			/*indices[pointer++] = topLeft;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = topRight;*/
			triangles.push_back(glm::uvec3{topLeft, bottomLeft, topRight});
			/*indices[pointer++] = topRight;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = bottomRight;*/
			triangles.push_back(glm::uvec3{ topRight, bottomLeft, bottomRight });
		}
	}

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("resources/benji.png", &texWidth, &texHeight, &texChannels, 3);

	GLuint texLight;
	glCreateTextures(GL_TEXTURE_2D, 1, &texLight);
	glTextureStorage2D(texLight, 1, GL_RGB8, texWidth, texHeight);
	glTextureSubImage2D(texLight, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range.
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation).
	glTextureParameteri(texLight, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texLight, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint framebuffer;
	glCreateFramebuffers(1, &framebuffer);
	glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, texLight, 0);

	ShaderBuilder terrainShader;
	terrainShader.addStage(GL_VERTEX_SHADER, "shaders/shader_terrain_vert.glsl");
	terrainShader.addStage(GL_FRAGMENT_SHADER, "shaders/shader_terrain_frag.glsl");
	

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{ -SIZE/2,0,-SIZE/4 });
	m_modelMatrix = translationMatrix;
	const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
	// Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
	// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
	const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));
	Shader terrainS = terrainShader.build();
	terrainS.bind();
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
	glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
	glUniform3fv(6, 1, glm::value_ptr(cameraPosition));
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texLight);
	glUniform1i(3, 0);

	




	GLuint VAO, VBO, IBO;
	
	glCreateBuffers(1, &IBO);
	glNamedBufferStorage(IBO, static_cast<GLsizeiptr>(triangles.size() * sizeof(decltype(triangles)::value_type)), triangles.data(), 0);

	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, static_cast<GLsizeiptr>(vertices2.size() * sizeof(decltype(vertices2)::value_type)), vertices2.data(), 0);

	// Bind vertex data to shader inputs using their index (location).
	// These bindings are stored in the Vertex Array Object.
	glCreateVertexArrays(1, &VAO);

	// The indices (pointing to vertices) should be read from the index buffer.
	glVertexArrayElementBuffer(VAO, IBO);

	// See definition of Vertex in <framework/mesh.h>
	// We bind the vertex buffer to slot 0 of the VAO and tell the VBO how large each vertex is (stride).
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(terrainVertex));
	// Tell OpenGL that we will be using vertex attributes 0, 1 and 2.
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);
	// We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, 0, offsetof(terrainVertex, position));
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, false, offsetof(terrainVertex, normal));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, false, offsetof(terrainVertex, texCoord));
	// For each of the vertex attributes we tell OpenGL to get them from VBO at slot 0.
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribBinding(VAO, 2, 0);
	glBindVertexArray(0);
	
	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, vertices2.size() / 3);
	glDrawElements(GL_TRIANGLES, triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
}


// not used, just copied into the main method above
void Terrain::terrainTexture() {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("resources/checkerboard.png", &texWidth, &texHeight, &texChannels, 3);

	GLuint texLight;
	glCreateTextures(GL_TEXTURE_2D, 1, &texLight);
	glTextureStorage2D(texLight, 1, GL_RGB8, texWidth, texHeight);
	glTextureSubImage2D(texLight, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range.
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation).
	glTextureParameteri(texLight, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texLight, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint framebuffer;
	glCreateFramebuffers(1, &framebuffer);
	glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, texLight, 0);

}



