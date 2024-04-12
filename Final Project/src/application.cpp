//#include "Image.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "terrain.h"
#include "animations.h"
#include "light.h"
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
#include <glm/gtx/string_cast.hpp>

DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>
#include <span>
#include <chrono>
#include <numbers>




class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL45)
        , m_texture1("resources/skybox/bottom.jpg"),
        m_texture2("resources/skybox/front.jpg"),
        m_texture3("resources/skybox/back.jpg"),
        m_texture4("resources/skybox/top.jpg"),
        m_texture5("resources/skybox/left.jpg"),
        m_texture6("resources/skybox/right.jpg"),
        m_billboard0("resources/animations/billBoard/0.png"),
        m_billboard1("resources/animations/billBoard/1.png"),
        m_billboard2("resources/animations/billBoard/2.png"),
        m_billboard3("resources/animations/billBoard/3.png"),
        m_billboard4("resources/animations/billBoard/4.png"),
        m_billboard5("resources/animations/billBoard/5.png"),
        m_billboard6("resources/animations/billBoard/6.png"),
        m_billboard7("resources/animations/billBoard/7.png")

        {
            carLocation = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };
            glm::quat meshOrientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            float distanceFromMesh = 40.0f; // How far the camera is from the mesh
            glm::vec3 cameraPosition = carLocation.position + meshOrientation * glm::vec3(0.0f, 5.0f, -distanceFromMesh);
            glm::vec3 direction = glm::normalize(carLocation.position - cameraPosition - glm::vec3(0.0f, -4.0f,0.0f));


            glm::quat meshOrientationTop = glm::angleAxis(glm::radians(89.0f), glm::vec3(1.f, 0.f, 0.0f));
            glm::vec3 cameraPositionTop = carLocation.position + meshOrientationTop * glm::vec3(0.0f, 0.0f, -distanceFromMesh);
            glm::vec3 directionTop = glm::normalize(carLocation.position - cameraPositionTop);


            anim_splines1 = loadSplinesFromJSON("resources/animations/BezierCurve1_data.json");
            std::cout << "loaded splines: " << !anim_splines1.empty() << std::endl;
            animTimer = 0.0f;
            animDuration = 50.0f;
            inAnimation = false;
            animNumber = 0;
            dayFactor = 1;
            Camera camera1{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
            Camera camera2{ &m_window, cameraPosition, direction };
            Camera camera3{ &m_window, cameraPositionTop, directionTop };
            Camera camera4{ &m_window, carLocation.position + meshOrientationTop * glm::vec3(0.0f, 0.0f, -distanceFromMesh ) , directionTop };

            topView = camera3;
            miniMap = camera4;
            light_camera = camera2;
            camera = camera1;
            temp = { &m_window };
            cam1 = true;
            top = false;
            moving = false;
            forward = false;
            centerSkybox = false;

            kd = false;
            bphong = false;


            procedural = false;
            light.addLight(glm::vec3(20, 1, 30), glm::vec3(1), glm::vec3{ 1,0.01f,0.002f });
            light.addLight(glm::vec3(-20, 1, 30), glm::vec3(1), glm::vec3{ 1,0.01f,0.002f });
            light.addLight(glm::vec3(0, 200, 0), glm::vec3{ 0.9922f, 0.7216f,  0.0745f }, glm::vec3{ 1,0.01f,0.002f });
            
            //terrain = Terrain();
            //transform = Transformation();
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
            skybox = GPUMesh::loadMeshGPU("resources/skybox.obj");
            arm = GPUMesh::loadMeshGPU("resources/cyliner.obj");
            sun = GPUMesh::loadMeshGPU("resources/sun.obj");
            billboard = GPUMesh::loadMeshGPU("resources/billboard.obj");

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

                ShaderBuilder skyboxBuilder;
                skyboxBuilder.addStage(GL_VERTEX_SHADER, "shaders/skybox_vert.glsl");
                skyboxBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/skybox_frag.glsl");
                m_skyboxShader = skyboxBuilder.build();

                ShaderBuilder environmentShader;
                environmentShader.addStage(GL_VERTEX_SHADER, "shaders/env_vert.glsl");
                environmentShader.addStage(GL_FRAGMENT_SHADER, "shaders/env_frag.glsl");
                m_environmentShader = environmentShader.build();

                ShaderBuilder sunShader;
                sunShader.addStage(GL_VERTEX_SHADER, "shaders/sun_vert.glsl");
                sunShader.addStage(GL_FRAGMENT_SHADER, "shaders/sun_frag.glsl");
                m_sunShader = sunShader.build();

                ShaderBuilder mmShader;
                mmShader.addStage(GL_VERTEX_SHADER, "shaders/mm_vert.glsl");
                mmShader.addStage(GL_FRAGMENT_SHADER, "shaders/mm_frag.glsl");
                m_minimapShader = mmShader.build();

                // Any new shaders can be added below in similar fashion.
                // ==> Don't forget to reconfigure CMake when you do!
                //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
                //     VS Code: ctrl + shift + p => CMake: Configure => enter
                // ....
            }
            catch (ShaderLoadingException e) {
                std::cerr << e.what() << std::endl;
            }

        };



    void update()
    {
        int dummyInteger = 0; // Initialized to 0
        
            
        m_texture1.bind(GL_TEXTURE1);
        m_texture2.bind(GL_TEXTURE2);
        m_texture3.bind(GL_TEXTURE3);
        m_texture4.bind(GL_TEXTURE4);
        m_texture5.bind(GL_TEXTURE5);
        m_texture6.bind(GL_TEXTURE6);

        m_billboard0.bind(GL_TEXTURE10);
        m_billboard1.bind(GL_TEXTURE11);
        m_billboard2.bind(GL_TEXTURE12);
        m_billboard3.bind(GL_TEXTURE13);
        m_billboard4.bind(GL_TEXTURE14);
        m_billboard5.bind(GL_TEXTURE15);
        m_billboard6.bind(GL_TEXTURE16);
        m_billboard7.bind(GL_TEXTURE17);
        //terrain.renderTerrain(thirdPersonView.viewMatrix(), camera.cameraPos());

        static GLfloat cubeVertices[] = {
            // positions          // normals
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };

        GLuint cubeVBO, cubeVAO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
        // Set vertex attribute pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        


        static std::vector<std::string> faces
        {

            ("resources/cubemap/back.jpg"),
            ("resources/cubemap/front.jpg"),
            ("resources/cubemap/top.jpg"),
            ("resources/cubemap/bottom.jpg"),
            ("resources/cubemap/right.jpg"),
            ("resources/cubemap/left.jpg"),
        };
;        

        
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrComponents;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        GLuint cubeMapTexture = textureID;

        
        auto startTime = std::chrono::system_clock::now();
        double elapsedTime = 0.0;
        auto now = std::chrono::system_clock::now();
        double rawValue = 0.0;
        double sunX = 0.0;
        double sunY = 0.0;
        int cycleduration = 30;
        glm::vec3 sunCol = glm::vec3{ 0.9922f, 0.7216f,  0.0745f };
        glm::quat meshOrientation;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);



        int billboardTexture = 10;


        unsigned int quadVAO, quadVBO;
        float quadVertices[] = {
            // Positions       // Texture coordinates
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f
        };

        // Create VAO and VBO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Unbind VAO and VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        GLuint minimapFramebuffer;
        glGenFramebuffers(1, &minimapFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, minimapFramebuffer);

        GLuint minimapTexture;
        glGenTextures(1, &minimapTexture);
        glBindTexture(GL_TEXTURE_2D, minimapTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapTexture, 0);
        

        


        /////////////MAIN LOOP////////////////


        while (!m_window.shouldClose()) {
            now = std::chrono::system_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
            rawValue = fmod(elapsedTime, cycleduration) / cycleduration;
            rawValue = rawValue < 0.5 ? rawValue * 2.0 : (1.0 - rawValue) * 2.0;
            dayFactor = 0.3 + (1.0 - 0.3) * rawValue;
            double angle = elapsedTime / (2 * cycleduration) * 2 * std::numbers::pi; // Angle in radians
            sunX = std::cos(angle);
            sunY = std::sin(angle);
            if (sunY < 0) {
                sunX *= -1;
                sunY *= -1;
            }
            
            Light s = Light(glm::vec3(sunX*150, sunY*150, 0), sunCol, glm::vec3{ 1, 0, 0});
            light.replace(s, 2);

            glm::vec3 positions[3];
            glm::vec3 attenuation[3];
            for (int i = 0; i < 3; i++) {
                positions[i] = light.returnLight()[i].returnPos();
            }
            for (int i = 0; i < 3; i++) {
                attenuation[i] = light.returnLight()[i].returnAttenuation();
            }
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
            glClearDepth(1.0f);

            // ...
            glEnable(GL_DEPTH_TEST);

            camera.updateInput();
            glm::vec3 shiftPos = glm::vec3(0.0f, 0.0f, 0.0f);
            
            if (!anim_splines1.empty()) {
                if (inAnimation) {
                    if (animTimer < animDuration - 0.1f) {
                        WorldPosition oldPos = getPointOnCompositeCurve(anim_splines1, 100.0f * animTimer / animDuration, animNumber);
                        animTimer = (animTimer + 0.1f);
                        WorldPosition pos = getPointOnCompositeCurve(anim_splines1, 100.0f * animTimer / animDuration, animNumber);
                        //calculate how much the car moved in the last update
                        if(animNumber == 1)
                    		carLocation.direction = -pos.direction;
                        else carLocation.direction = pos.direction;
                        shiftPos = pos.position - oldPos.position;
                        carLocation.position += shiftPos;
                    }
                    else if (animNumber < anim_splines1.size()-1)
                    {
                        animTimer = 0.0f;
                        animNumber++;
                    }
                    else{
                        inAnimation = false;
                        animTimer = 0.0f;
                        animNumber = 0;
                        std::cout << "animation finished" << std::endl;
                    }
                }
            } else {
            	std::cout << "No animation data loaded" << std::endl;
            }

            float distanceFromMesh = 40.0f; // How far the camera is from the mesh
            glm::vec3 cameraPosition = carLocation.position + meshOrientation * glm::vec3(0.0f, 0.0f, -distanceFromMesh);
            glm::vec3 direction = glm::normalize(carLocation.position - cameraPosition);

            //car orientation
            meshOrientation = getCarOrientation(carLocation.direction, glm::vec3(0.0f, 1.0f, 0.0f));


            // Calculate the right vector (cross product of direction and up)
            glm::vec3 right = glm::normalize(glm::cross(up, direction));

            // Recalculate the up vector (cross product of right and direction)
            up = glm::cross(direction, right);

            // Create the view matrix
            glm::mat4 viewMatrix = glm::lookAt(cameraPosition, carLocation.position, up);
            
            if (moving) {
                if (forward) {
                    carLocation.position.z += 1.5f;
                    topView.changePos(glm::vec3{topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z + 1.5});
                    miniMap.changePos(glm::vec3{ topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z + 1.5 });
                }
                else {
                    carLocation.position.z -= 1.5f;
                    topView.changePos(glm::vec3{ topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z - 1.5});
                    miniMap.changePos(glm::vec3{ topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z - 1.5 });
                }
            }
            //SET CAMERA POSITION AND DIRECTION
            light_camera.changePos(carLocation.position + meshOrientation * glm::vec3(1.0f, 5.0f, -distanceFromMesh));
            light_camera.changeDir(meshOrientation, false);
            topView.changePos(topView.cameraPos() + shiftPos);

            glm::mat4 view = camera.viewMatrix();

            if (top) {
               
                view = topView.viewMatrix();
            }
            else if (!cam1) {
                view = light_camera.viewMatrix();
            }

            //TRANSLATE CAR MODEL ACCORDING TO ANIMATION
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{carLocation.position});
            glm::mat4 rotationMatrix = glm::mat4_cast(meshOrientation);

            m_modelMatrix = translationMatrix * rotationMatrix;
            const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;


            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, 0.1f, 100.0f);
            



            GLint origViewport[4];
            glGetIntegerv(GL_VIEWPORT, origViewport);

            view = miniMap.viewMatrix();
            glBindFramebuffer(GL_FRAMEBUFFER, minimapFramebuffer);
            glViewport(0, 0, 1024, 1024); // Set viewport to match texture size
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            const glm::mat4 miniMVP = m_projectionMatrix * view * m_modelMatrix;

            {
                for (GPUMesh& mesh : m_meshes) {

                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(miniMVP));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));



                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);

                }

                ///////////////////////////////////////////////////// BILLBOARD //////////////////////////////////////////////////////////////////////////

                glm::mat4 billBoardTranslationMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3{ -14    , -2.5, 50 }), glm::vec3(2));
                glm::mat4 boardRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 boardModelMatrix = billBoardTranslationMatrix * boardRotationMatrix;

                const glm::mat4 billBoardMVP = m_projectionMatrix * view * boardModelMatrix;

                for (GPUMesh& mesh : billboard) {
                    if (mesh.hasTextureCoords()) {
                        if (billboardTexture == 18) billboardTexture = 10;

                        m_defaultShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(billBoardMVP));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(boardModelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                        glUniform1i(3, billboardTexture);
                        glUniform1i(4, GL_TRUE);
                        glUniform1i(5, GL_FALSE);
                        mesh.draw(m_defaultShader);

                        billboardTexture++;
                    }
                    else {
                        m_bphongShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(billBoardMVP));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(boardModelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                        glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                        glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                        mesh.draw(m_bphongShader);
                    }
                }



                ///////////////////////////////////////////////////// START ROBOT ARM //////////////////////////////////////////////////////////////////////////

                glm::mat4 translationMatrixArm1 = glm::translate(glm::mat4(1.0f), glm::vec3{ 15,0,0 });
                for (GPUMesh& mesh : arm) {


                    m_modelMatrix = translationMatrixArm1;
                    const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;

                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);


                }


                for (int i = 0; i < 3; i++) {
                    for (GPUMesh& mesh : arm) {
                        tracker = (tracker + 1) % 9;
                        rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(float(arr[tracker])), glm::vec3(0, 0, 1));

                        translationMatrixArm1 = translationMatrixArm1 * glm::translate(glm::mat4(1.0f), glm::vec3{ -0.5,4.5,0 }) * rotationS;
                        m_modelMatrix = translationMatrixArm1;
                        const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;

                        m_bphongShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                        glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                        glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                        mesh.draw(m_bphongShader);


                    }

                }

                glm::vec4 newLightPos = glm::vec4(light.returnLight()[0].returnPos(), 1);
                newLightPos = translationMatrixArm1 * glm::translate(glm::mat4(1.0f), glm::vec3{ -0.5,4.5,0 }) * glm::vec4(1);


                Light l = Light(newLightPos, glm::vec3(1), light.returnLightIndex(0).returnAttenuation());
                light.replace(l, 0);


                glm::mat4 translationMatrixArm2 = glm::translate(glm::mat4(1.0f), glm::vec3{ -15,0,0 });
                for (GPUMesh& mesh : arm) {
                    m_modelMatrix = translationMatrixArm2;
                    const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;

                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);
                }


                for (int i = 0; i < 3; i++) {
                    for (GPUMesh& mesh : arm) {
                        tracker = (tracker + 1) % 9;
                        rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(-float(arr[tracker])), glm::vec3(0, 0, 1));

                        translationMatrixArm2 = translationMatrixArm2 * glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5,4.5,0 }) * rotationS;
                        m_modelMatrix = translationMatrixArm2;
                        const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;

                        m_bphongShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                        glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                        glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                        mesh.draw(m_bphongShader);
                    }

                }


                //terrain.renderTerrain(view, light.returnLight(), procedural, terrainLevel);
            }
            glViewport(origViewport[0], origViewport[1], (GLsizei)origViewport[2], (GLsizei)origViewport[3]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            view = camera.viewMatrix();

            if (top) {

                view = topView.viewMatrix();
            }
            else if (!cam1) {
                view = light_camera.viewMatrix();
            }
           

            for (GPUMesh& mesh : m_meshes) {

                m_bphongShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                

                glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                mesh.draw(m_bphongShader);

            }

            ///////////////////////////////////////////////////// BILLBOARD //////////////////////////////////////////////////////////////////////////

            glm::mat4 billBoardTranslationMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3{ -14    , -2.5, 50 }), glm::vec3(2));
            glm::mat4 boardRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 boardModelMatrix = billBoardTranslationMatrix * boardRotationMatrix;
            
            const glm::mat4 billBoardMVP = m_projectionMatrix * view * boardModelMatrix;

            for (GPUMesh& mesh : billboard) {
                if (mesh.hasTextureCoords()) {
                    if (billboardTexture == 18) billboardTexture = 10;

                    m_defaultShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(billBoardMVP));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(boardModelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                    glUniform1i(3, billboardTexture);
                    glUniform1i(4, GL_TRUE);
                    glUniform1i(5, GL_FALSE);
                    mesh.draw(m_defaultShader);

                    billboardTexture++;
                }
                else {
                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(billBoardMVP));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(boardModelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);
                }
            }
                

                
                 ///////////////////////////////////////////////////// START ROBOT ARM //////////////////////////////////////////////////////////////////////////

            glm::mat4 translationMatrixArm1 = glm::translate(glm::mat4(1.0f), glm::vec3{ 15,0,0 });
            for (GPUMesh& mesh : arm) {


                m_modelMatrix = translationMatrixArm1;
                const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                    
                m_bphongShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                mesh.draw(m_bphongShader);

                        
            }


            for (int i = 0; i < 3; i++) {
                for (GPUMesh& mesh : arm) {
                    tracker = (tracker + 1) % 9;
                    rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(float(arr[tracker])), glm::vec3(0, 0, 1));

                    translationMatrixArm1 = translationMatrixArm1 * glm::translate(glm::mat4(1.0f), glm::vec3{ -0.5,4.5,0 }) * rotationS;
                    m_modelMatrix = translationMatrixArm1;
                    const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                        
                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);

                            
                }

            }

            glm::vec4 newLightPos = glm::vec4(light.returnLight()[0].returnPos(), 1);
            newLightPos = translationMatrixArm1 * glm::translate(glm::mat4(1.0f), glm::vec3{ -0.5,4.5,0 }) * glm::vec4(1);


            Light l = Light(newLightPos, glm::vec3(1), light.returnLightIndex(0).returnAttenuation());
            light.replace(l, 0);
               

            glm::mat4 translationMatrixArm2 = glm::translate(glm::mat4(1.0f), glm::vec3{ -15,0,0 });
            for (GPUMesh& mesh : arm) {
                m_modelMatrix = translationMatrixArm2;
                const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                    
                m_bphongShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                mesh.draw(m_bphongShader);                
            }


            for (int i = 0; i < 3; i++) {
                for (GPUMesh& mesh : arm) {
                    tracker = (tracker + 1) % 9;
                    rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(-float(arr[tracker])), glm::vec3(0, 0, 1));

                    translationMatrixArm2 = translationMatrixArm2 * glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5,4.5,0 }) * rotationS;
                    m_modelMatrix = translationMatrixArm2;
                    const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                        
                    m_bphongShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                    glUniform3fv(6, 3, glm::value_ptr(positions[0]));
                    glUniform3fv(11, 3, glm::value_ptr(attenuation[0]));

                    mesh.draw(m_bphongShader);
                }

            }

            glm::vec4 newLightPos2;
            newLightPos2 = translationMatrixArm2 * glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5,4.5,0 }) * glm::vec4(1);


            Light l2 = Light(newLightPos2, glm::vec3(1), light.returnLightIndex(1).returnAttenuation());
            light.replace(l2, 1);
                


            ///////////////////////////////////////////////// END ROBOT ARM ////////////////////////////////////////////////////////////////////////////////////////


                //center skybox on camera or car
                glm::mat4 skyModelMatrix = glm::translate(glm::mat4(1.0f), camera.cameraPos());
                if (centerSkybox) {
				    skyModelMatrix = glm::translate(glm::mat4(1.0f), carLocation.position);
                }
                const glm::mat4 mvpMatrixSky = m_projectionMatrix * view * skyModelMatrix;


            for (int i = 0; i < 6; i++) {
                GPUMesh& mesh = skybox[i];
                m_skyboxShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixSky));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(skyModelMatrix));
                glUniform1i(3, i + 1);
                   
                glUniform1f(7, dayFactor);
                mesh.draw(m_skyboxShader);
            }

            m_environmentShader.bind();
            glBindVertexArray(0);
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));
            glUniform3fv(3, 1, glm::value_ptr(camera.cameraPos()));
            glUniform1f(4, dayFactor);
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);


            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Render other objects with environment mapping

            glm::mat4 sunModelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), light.returnLight()[2].returnPos()), glm::vec3(4.0));
            const glm::mat4 sunMVP = m_projectionMatrix * view * sunModelMatrix;

            for (GPUMesh& mesh : sun) {
                m_sunShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(sunMVP));
                mesh.draw(m_sunShader);
            }
                



           

            ///////////////////////////////////////////////// MiniMap ////////////////////////////////////////////////////////////////////////////////////////
            

            
            glm::mat4 mmview = glm::mat4(1.0f); // Identity matrix for view
            glm::mat4 mmmodel = glm::scale(glm::translate(glm::mat4(1.0f),  glm::vec3(0.7,0.7,-1.1)), glm::vec3(0.2f)); 

            glm::mat4 mmmvpMatrix = m_projectionMatrix * mmview * mmmodel;


            m_minimapShader.bind();
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mmmvpMatrix));

            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, minimapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
          

            terrain.renderTerrain(view, light.returnLight(), procedural, terrainLevel);

            if (terrainLevel * 800 + 400 < carLocation.position.z) {
                terrainLevel++;
            }

            if (terrainLevel * 800 - 400 > carLocation.position.z) {
                terrainLevel--;
            }
            //////////////////////////
        
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
            if (!cam1 || top) {
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = true;
                top = false;
                centerSkybox = false;
            }

            break;
        case GLFW_KEY_2:
            if (cam1 || top) {
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = false;
                top = false;
                centerSkybox = true;
            }
            break;

        case GLFW_KEY_3:    
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = false;
                top = true;
                centerSkybox = true;

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
        case GLFW_KEY_C:
            inAnimation = !inAnimation;
            std::cout << "playing animation: " << inAnimation << std::endl;
            break;

        case GLFW_KEY_T:
            procedural = !procedural;
            
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
        //std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
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
    Shader m_skyboxShader;
    Shader m_environmentShader;
    Shader m_sunShader;
    Shader m_minimapShader;


    unsigned int loadCubemap(std::vector<std::string> faces);


    std::vector<GPUMesh> m_meshes;
    std::vector<GPUMesh> road;
    std::vector<GPUMesh> skybox;
    std::vector<GPUMesh> arm;
    std::vector<GPUMesh> sun;
    std::vector<GPUMesh> billboard;

    Texture m_texture1;
    Texture m_texture2;
    Texture m_texture3;
    Texture m_texture4;
    Texture m_texture5;
    Texture m_texture6;
    Texture m_billboard0;
    Texture m_billboard1;
    Texture m_billboard2;
    Texture m_billboard3;
    Texture m_billboard4;
    Texture m_billboard5;
    Texture m_billboard6;
    Texture m_billboard7;
    bool m_useMaterial { true };
    Camera camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera topView{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera light_camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera miniMap{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera temp{ &m_window};
    bool cam1{ true };
    bool top{ false };
    Terrain terrain;
    Terrain terrain2;
    float animTimer;
    float animDuration;
    int animNumber;
    std::vector <BezierSpline> anim_splines1;
    WorldPosition carLocation;
    float move;
    bool moving;
    bool forward;
    bool kd;
    bool bphong;
    bool inAnimation;
    bool centerSkybox;
    bool procedural;
    float dayFactor;
    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 1.1f, 500.f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };

    int arr[9] = {8, 10, 12, 15, 20 ,15,12,10,8 };
    int tracker = 0;
    int slower = 0;
    int terrainLevel = 0;
    glm::mat4 rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(10.f), glm::vec3(0, 0, 1));
    glm::mat4 scale1 = glm::scale(glm::mat4(1.0f), glm::vec3(2, 1, 1));
    glm::mat4 unscale = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));
    /*struct Light {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 attenuation;
    };
    std::vector<Light> lights{ Light { glm::vec3(20,5, 30), glm::vec3(1), glm::vec3{1,0.1f,0.02f} }, Light { glm::vec3(-20,5, -30), glm::vec3(1), glm::vec3{1,0.1f,0.02f} } };*/
    Light light;
    
};



int main()
{
    Application app;
    app.update();

    return 0;
}

unsigned int Application::loadCubemap(std::vector<std::string> faces)
{
    return 0;
}


