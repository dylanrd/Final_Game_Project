// Suppress warnings in third-party code.
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
DISABLE_WARNINGS_POP()
#include <framework/window.h>

struct terrainVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord; // Texture coordinate

	[[nodiscard]] constexpr bool operator==(const terrainVertex&) const noexcept = default;
};

class Terrain {
public:
	Terrain(void);
    void renderTerrain(glm::mat4 view, glm::vec3 cameraPosition);
	void terrainTexture();

private:
	glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 1.1f, 500.f);
	glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
	glm::mat4 m_modelMatrix{ 1.0f };

private:
    
};