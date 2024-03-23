//#include "Image.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "terrain.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>

class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL45)
        , m_texture("resources/Body21_diff.png")
    {
        carPosition = { 0.f, 0.f, 0.f };
        glm::quat meshOrientation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.5f, 0.f, 0.0f));
        float distanceFromMesh = 40.0f; // How far the camera is from the mesh
        glm::vec3 cameraPosition = carPosition + meshOrientation * glm::vec3(0.0f, 0.0f, -distanceFromMesh);
        glm::vec3 direction = glm::normalize(carPosition - cameraPosition);
        
        Camera camera1{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
        Camera camera2{ &m_window, cameraPosition, direction };
        thirdPersonView = camera2;
        light_camera = camera2;
        camera = camera1;
        temp = { &m_window };
        cam1 = true;
        move = 0.f;
        moving = false;
        forward = false;

        kd = false;
        bphong = false;
        //terrain = Terrain();

        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods);

            
        });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods);
        });

        m_meshes = GPUMesh::loadMeshGPU("resources/carTexturesTest.obj");
        road = GPUMesh::loadMeshGPU("resources/temp_road.obj");

        try {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

      
            m_bphongShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/blinn_phong_frag.glsl").build();

            ShaderBuilder kdShaderBuilder;
            kdShaderBuilder.addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl");
            kdShaderBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/lambertKd_frag.glsl");
            m_kdShader = kdShaderBuilder.build();

            

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, "shaders/shadow_vert.glsl");
            m_shadowShader = shadowBuilder.build();

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void update()
    {
        int dummyInteger = 0; // Initialized to 0
     
      
        //terrain.renderTerrain(thirdPersonView.viewMatrix(), camera.cameraPos());

        while (!m_window.shouldClose()) {
            

            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();

           

            // Use ImGui for easy input/output of ints, floats, strings, etc...
            ImGui::Begin("Window");
            ImGui::InputInt("This is an integer input", &dummyInteger); // Use ImGui::DragInt or ImGui::DragFloat for larger range of numbers.
            ImGui::Text("Value is: %i", dummyInteger); // Use C printf formatting rules (%i is a signed integer)
            ImGui::Checkbox("Use material if no texture", &m_useMaterial);
            ImGui::End();

            // Clear the screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...
            glEnable(GL_DEPTH_TEST);

            camera.updateInput();
            glm::quat meshOrientation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            float distanceFromMesh = 10.0f; // How far the camera is from the mesh
            glm::vec3 cameraPosition = carPosition + meshOrientation * glm::vec3(0.0f, 0.0f, -distanceFromMesh);
            glm::vec3 direction = glm::normalize(carPosition - cameraPosition);

            // Calculate the up vector (usually (0, 1, 0) for a top-down view)
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            // Calculate the right vector (cross product of direction and up)
            glm::vec3 right = glm::normalize(glm::cross(up, direction));

            // Recalculate the up vector (cross product of right and direction)
            up = glm::cross(direction, right);

            // Create the view matrix
            glm::mat4 viewMatrix = glm::lookAt(cameraPosition, carPosition, up);
            if (moving) {
                if (forward) {
                    move = move + 0.2f;
                    carPosition.z += 0.2f;
                    light_camera.changePos(glm::vec3{ light_camera.cameraPos().x, light_camera.cameraPos().y, light_camera.cameraPos().z + 0.2 });
                }
                else {
                    move = move - 0.2f;
                    carPosition.z -= 0.2f;
                    light_camera.changePos(glm::vec3{ light_camera.cameraPos().x, light_camera.cameraPos().y, light_camera.cameraPos().z - 0.2 });
                }
            }

            glm::mat4 view = camera.viewMatrix();

            if (!cam1) {
                view = light_camera.viewMatrix();
            }
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{ 0,0,move });
            m_modelMatrix = translationMatrix;
            const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            for (GPUMesh& mesh : m_meshes) {
                if (!kd && !bphong) {
                    m_defaultShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                    if (mesh.hasTextureCoords()) {
                        m_texture.bind(GL_TEXTURE0);
                        glUniform1i(3, 0);
                        glUniform1i(4, GL_TRUE);
                        glUniform1i(5, GL_FALSE);
                    }
                    else {
                        glUniform1i(4, GL_FALSE);
                        glUniform1i(5, m_useMaterial);
                        glUniform3fv(6, 1, glm::value_ptr(camera.cameraPos()));
                    }
                    mesh.draw(m_defaultShader);
                   
                }
                else {
                    if (kd) {
                        std::cout << "here" << std::endl;
                        m_kdShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                        glUniform1i(4, GL_FALSE);
                        glUniform1i(5, m_useMaterial);
                        glUniform3fv(6, 1, glm::value_ptr(camera.cameraPos()));

                        mesh.draw(m_kdShader);
                    }
                    else {
                        m_bphongShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                        glUniform1i(4, GL_FALSE);
                        glUniform1i(5, m_useMaterial);
                        glUniform3fv(6, 1, glm::value_ptr(camera.cameraPos()));

                        mesh.draw(m_bphongShader);
                        
                    }
                }
                
            }

           /* glm::mat4 translationMatrixGround = glm::translate(glm::mat4(1.0f), glm::vec3{ 0,-3,0 });
            for (GPUMesh& mesh : road) {
                m_defaultShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * view * translationMatrixGround));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(translationMatrixGround));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(translationMatrixGround)));
                if (mesh.hasTextureCoords()) {
                    m_texture.bind(GL_TEXTURE0);
                    glUniform1i(3, 0);
                    glUniform1i(4, GL_TRUE);
                    glUniform1i(5, GL_FALSE);
                }
                else {
                    glUniform1i(4, GL_FALSE);
                    glUniform1i(5, m_useMaterial);
                }
                mesh.draw(m_defaultShader);
            }*/

            //terrain.renderTerrain(view, camera.cameraPos());
            
            m_window.swapBuffers();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        switch (key) {
        case GLFW_KEY_1:
            if (!cam1) {
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = true;
            }

            break;
        case GLFW_KEY_2:
            if (cam1) {
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = false;
            }
            break;

        case GLFW_KEY_UP:
            moving = true;
            forward = true;
            break;
        case GLFW_KEY_DOWN:
            moving = true;
            forward = false;
            break;
        case GLFW_KEY_P:
            kd = !kd;
            std::cout << "Kd is " << kd << std::endl;
            break;
        case GLFW_KEY_O:
            bphong = !bphong;
            std::cout << "blinn phong is " << bphong << std::endl;
            break;
        default:
            break;
        }
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
        switch (key) {
        case GLFW_KEY_UP:
            moving = false;
            break;
        case GLFW_KEY_DOWN:
            moving = false;
            break;
        }
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos)
    {
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        std::cout << "Pressed mouse button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {
        std::cout << "Released mouse button: " << button << std::endl;
    }

private:
    Window m_window;

    // Shader for default rendering and for depth rendering
    Shader m_defaultShader;
    Shader m_kdShader;
    Shader m_bphongShader;
    Shader m_shadowShader;

    std::vector<GPUMesh> m_meshes;
    std::vector<GPUMesh> road;
    Texture m_texture;
    bool m_useMaterial { true };
    Camera camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera thirdPersonView{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera light_camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera temp{ &m_window};
    bool cam1{ true };
    glm::vec3 carPosition;
    Terrain terrain;

    float move;
    bool moving;
    bool forward;
    bool kd;
    bool bphong;
    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 1.1f, 500.f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };
};

int main()
{
    Application app;
    app.update();

    return 0;
}
