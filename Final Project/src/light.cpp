#include "light.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()


Light::Light(void)
    
{

}

Light::Light(glm::vec3 position, glm::vec3 colour, glm::vec3 attenuation)
    : m_position(position)
    , m_colour(colour)
    , m_attenuation(attenuation)
{
    
}

void Light::addLight(glm::vec3 position, glm::vec3 colour, glm::vec3 attenuation) {
    lights.push_back(Light(position, colour, attenuation));
}


std::vector<Light> Light::returnLight() {
    return lights;
}

Light Light::returnLightIndex(int index) {
    return lights[index];
}


void Light::changePos(const glm::vec3& pos)
{
    m_position = pos;
}

void Light::replace(Light& light, int index) {
    lights[index] = light;
}

glm::vec3 Light::returnPos() {
    return m_position;
}

void Light::changeAttenuation(const glm::vec3& att)
{
    m_attenuation = att;
}

glm::vec3 Light::returnAttenuation() {
    return m_attenuation;
}

int Light::getSize() {
    return lights.size();
}


