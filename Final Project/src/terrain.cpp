#include "terrain.h"

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>
DISABLE_WARNINGS_POP()
#include <vector>
#include <framework/shader.h>
#include <iostream>




Terrain::Terrain(void)
    
{
}

void Terrain::renderTerrain(glm::mat4 view, std::vector<Light> lights, bool procedural) {
	int SIZE = 800;
	const int VERTEX_COUNT = 128;
	int MAX_HEIGHT = 20;
	int MIN_HEIGHT = -5;


	int heightWidth, heightHeight, heightChannels;
	stbi_uc* height_map = stbi_load("resources/Gravel_001_Height.png", &heightWidth, &heightHeight, &heightChannels, 3);
	
	
	int vertexPointer = 0;
	std::vector<terrainVertex> vertices2;
	std::vector<glm::uvec3> triangles;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			float vert1 = (float)j / ((float)VERTEX_COUNT - 1) * SIZE;
			float vert2;
			if (procedural) {
				//hG.changeSeed();
				if (!hGcreated) {
					hgBase.changeSeed();
					hGcreated = true;
					
				}
				
					vert2 = hgBase.generateHeight(i, j);
				
				
			}
			else {
				vert2 = 0;
				hGcreated = false;
			}
			
			float vert3 = (float)i / ((float)VERTEX_COUNT - 1) * SIZE;
			
			
			float text1 = (float) j / ((float)VERTEX_COUNT - 1);
			float text2 = (float) i / ((float)VERTEX_COUNT - 1);
			vertexPointer++;

			terrainVertex vertex{
						.position = {vert1,vert2,vert3},
						.normal = {0, 1, 0},
						.texCoord = {text1, text2}
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
			
			triangles.push_back(glm::uvec3{topLeft, bottomLeft, topRight});
			
			triangles.push_back(glm::uvec3{ topRight, bottomLeft, bottomRight });
		}
	}

	for (unsigned int i = 0; i < triangles.size(); i += 3) {

		terrainVertex& v0 = vertices2[triangles[i].x];
		terrainVertex& v1 = vertices2[triangles[i].y];
		terrainVertex& v2 = vertices2[triangles[i].z];

		glm::vec3 Edge1 = v1.position - v0.position;
		glm::vec3 Edge2 = v2.position - v0.position;

		double DeltaU1 = v1.texCoord.x - v0.texCoord.x;
		double DeltaV1 = v1.texCoord.y - v0.texCoord.y;
		double DeltaU2 = v2.texCoord.x - v0.texCoord.x;
		double DeltaV2 = v2.texCoord.y - v0.texCoord.y;

		float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		
		glm::vec3 Tangent, Bitangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Bitangent.x = f * (-DeltaU2 * Edge1.x + DeltaU1 * Edge2.x);
		Bitangent.y = f * (-DeltaU2 * Edge1.y + DeltaU1 * Edge2.y);
		Bitangent.z = f * (-DeltaU2 * Edge1.z + DeltaU1 * Edge2.z);


	
		v0.tangent += Tangent;
		v1.tangent += Tangent;
		v2.tangent += Tangent;
	}

	for (unsigned int i = 0; i < vertices2.size(); i++) {
		vertices2[i].tangent = glm::normalize(vertices2[i].tangent);
	}

	int texWidth, texHeight, texChannels;
	/*stbi_uc* pixels = stbi_load("resources/gravel.png", &texWidth, &texHeight, &texChannels, 3);*/
	int normalWidth, normalHeight, normalChannels;


	stbi_uc* pixels = stbi_load("resources/Gravel_001_BaseColor.jpg", &texWidth, &texHeight, &texChannels, 3);

	stbi_uc* normal_map = stbi_load("resources/Gravel_001_Normal.jpg", &normalWidth, &normalHeight, &normalChannels, 3);

	

	GLuint texLight;
	glCreateTextures(GL_TEXTURE_2D, 1, &texLight);
	glTextureStorage2D(texLight, 1, GL_RGB8, texWidth, texHeight);
	glTextureSubImage2D(texLight, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range.
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texLight, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation).
	glTextureParameteri(texLight, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texLight, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint normalMap;
	glCreateTextures(GL_TEXTURE_2D, 1, &normalMap);
	glTextureStorage2D(normalMap, 1, GL_RGB8, normalWidth, normalHeight);
	glTextureSubImage2D(normalMap, 0, 0, 0, normalWidth, normalHeight, GL_RGB, GL_UNSIGNED_BYTE, normal_map);

	// Set behaviour for when texture coordinates are outside the [0, 1] range.
	glTextureParameteri(normalMap, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(normalMap, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation).
	glTextureParameteri(normalMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(normalMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint framebuffer;
	glCreateFramebuffers(1, &framebuffer);
	glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, texLight, 0);


	GLuint framebuffer2;
	glCreateFramebuffers(1, &framebuffer2);
	glNamedFramebufferTexture(framebuffer2, GL_DEPTH_ATTACHMENT, normalMap, 0);

	ShaderBuilder terrainShader;
	terrainShader.addStage(GL_VERTEX_SHADER, "shaders/shader_terrain_vert.glsl");
	terrainShader.addStage(GL_FRAGMENT_SHADER, "shaders/shader_terrain_frag.glsl");
	

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{ -SIZE/2,-2.5,-SIZE/4 });
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
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texLight);
	glUniform1i(3, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glUniform1i(4, 1);


	glm::vec3 positions[2];
	
	for (int i = 0; i < 2; i++) {
		positions[i] = lights[i].returnPos();
	}
	
	glUniform3fv(7, 2, glm::value_ptr(positions[0]));

	glm::vec3 attenuation[2];

	for (int i = 0; i < 2; i++) {
		attenuation[i] = lights[i].returnAttenuation();
	}

	glUniform3fv(11, 2, glm::value_ptr(attenuation[0]));

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
	glEnableVertexArrayAttrib(VAO, 3);

	// We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, 0, offsetof(terrainVertex, position));
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, false, offsetof(terrainVertex, normal));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, false, offsetof(terrainVertex, texCoord));
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, 0, offsetof(terrainVertex, tangent));
	// For each of the vertex attributes we tell OpenGL to get them from VBO at slot 0.
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribBinding(VAO, 2, 0);
	glVertexArrayAttribBinding(VAO, 3, 0);
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



