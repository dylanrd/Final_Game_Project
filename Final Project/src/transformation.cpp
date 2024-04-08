#include "transformation.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()

DISABLE_WARNINGS_POP()
#include <vector>
#include <framework/shader.h>
#include "mesh.h"
#include <iostream>



Transformation::Transformation()

{
	/*defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
	defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
	m_defaultShader = defaultBuilder.build();

	m_bphongShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/blinn_phong_frag.glsl").build();

	kdShaderBuilder.addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl");
	kdShaderBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/lambertKd_frag.glsl");
	m_kdShader = kdShaderBuilder.build();*/
}

void Transformation::renderCylinder(glm::mat4 view, glm::vec3 cameraPosition) {
    
    Mesh fin = mergeMeshes(loadMesh("resources/cyliner.obj", true));


	ShaderBuilder cylinderShader;
	
	cylinderShader.addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/blinn_phong_frag.glsl");

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{10,100,0 });
	m_modelMatrix = translationMatrix;
	const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
	// Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
	// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
	const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));
	Shader terrainS = cylinderShader.build();
	terrainS.bind();
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
	glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
	glUniform3fv(6, 1, glm::value_ptr(cameraPosition));

	
	Shader cylShader = cylinderShader.build();
	cylShader.bind();
    
	

	std::vector<GPUMesh> fin2 = GPUMesh::loadMeshGPU("resources/cyliner.obj");
	
	for (GPUMesh& m : fin2) {
		
		m.draw(cylShader);
	}


	//GLuint VAO, VBO, IBO;
	//glCreateBuffers(1, &IBO);
	//glNamedBufferStorage(IBO, static_cast<GLsizeiptr>(fin.triangles.size() * sizeof(decltype(fin.triangles)::value_type)), fin.triangles.data(), 0);

	//glCreateBuffers(1, &VBO);
	//glNamedBufferStorage(VBO, static_cast<GLsizeiptr>(fin.vertices.size() * sizeof(decltype(fin.vertices)::value_type)), fin.vertices.data(), 0);

	//// Bind vertex data to shader inputs using their index (location).
	//// These bindings are stored in the Vertex Array Object.
	//glCreateVertexArrays(1, &VAO);

	//// The indices (pointing to vertices) should be read from the index buffer.
	//glVertexArrayElementBuffer(VAO, IBO);

	//// See definition of Vertex in <framework/mesh.h>
	//// We bind the vertex buffer to slot 0 of the VAO and tell the VBO how large each vertex is (stride).
	//glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	//// Tell OpenGL that we will be using vertex attributes 0, 1 and 2.
	//glEnableVertexArrayAttrib(VAO, 0);
	//glEnableVertexArrayAttrib(VAO, 1);
	//glEnableVertexArrayAttrib(VAO, 2);

	//// We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
	//glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, 0, offsetof(Vertex, position));
	//glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
	//glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, false, offsetof(Vertex, texCoord));

	//// For each of the vertex attributes we tell OpenGL to get them from VBO at slot 0.
	//glVertexArrayAttribBinding(VAO, 0, 0);
	//glVertexArrayAttribBinding(VAO, 1, 0);
	//glVertexArrayAttribBinding(VAO, 2, 0);
	//glBindVertexArray(0);

	//glBindVertexArray(VAO);
	//
	//glDrawElements(GL_TRIANGLES, fin.triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
    

    
    
}