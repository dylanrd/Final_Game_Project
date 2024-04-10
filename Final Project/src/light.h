#include <framework/disable_all_warnings.h>
#include <framework/window.h>
#include "glm/fwd.hpp"
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#ifndef LIGHT_H
#define LIGHT_H

class Light {
public:
    
    Light(void);
    Light(glm::vec3 position, glm::vec3 colour, glm::vec3 attenuation);

    std::vector<Light> returnLight();
    void addLight(glm::vec3 position, glm::vec3 colour, glm::vec3 attenuation);
    int getSize();
    Light* returnAllPos();
    void changePos(const glm::vec3& pos);
    void changeAttenuation(const glm::vec3& att);
   



    glm::vec3 returnPos();
    glm::vec3 returnAttenuation();
   

private:
    void rotateX(float angle);
    void rotateY(float angle);

private:
    static constexpr glm::vec3 s_yAxis{ 0, 1, 0 };
    glm::vec3 m_position{ 0 };
    glm::vec3 m_colour{ 0, 0, -1 };
    glm::vec3 m_up{ 0, 1, 0 };

    glm::vec3 m_attenuation{1, 0, 0};
    bool m_userInteraction{ true };
    glm::dvec2 m_prevCursorPos{ 0 };
    std::vector<Light> lights;
};

#endif LIGHT_H
