#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <stb/stb_image.h>
#include <framework/shader.h>
DISABLE_WARNINGS_POP()
#include <framework/window.h>
#include "mesh.h"


class Transformation {
public:
	Transformation();
	void renderCylinder(glm::mat4 view, glm::vec3 cameraPosition);
	//void terrainTexture();

private:
	glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 1.1f, 500.f);
	glm::mat4 m_modelMatrix{ 1.0f };

	Shader m_defaultShader;
	Shader m_kdShader;
	Shader m_bphongShader;

	std::vector<GPUMesh> cylinder = GPUMesh::loadMeshGPU("resources/skybox.obj");
	ShaderBuilder defaultBuilder;




	ShaderBuilder kdShaderBuilder;

};