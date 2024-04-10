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

unsigned int loadCubemap(std::vector<std::string> faces);



class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL45)
        , m_texture1("resources/skybox/bottom.jpg"), 
        m_texture2("resources/skybox/front.jpg"),
        m_texture3("resources/skybox/back.jpg"),
        m_texture4("resources/skybox/top.jpg"),
        m_texture5("resources/skybox/left.jpg"),
            m_texture6("resources/skybox/right.jpg")
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
            animDuration = 10.0f;
            inAnimation = false;
            animNumber = 0;

            Camera camera1{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
            Camera camera2{ &m_window, cameraPosition, direction };
            Camera camera3{ &m_window, cameraPositionTop, directionTop };
            topView = camera3;
            light_camera = camera2;
            camera = camera1;
            temp = { &m_window };
            cam1 = true;
            top = false;
            moving = false;
            forward = false;

            kd = false;
            bphong = false;

            light.addLight(glm::vec3(20, 1, 30), glm::vec3(1), glm::vec3{ 1,0.01f,0.002f });
            light.addLight(glm::vec3(-20, 1, 30), glm::vec3(1), glm::vec3{ 1,0.01f,0.002f });
            
            
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
        //terrain.renderTerrain(thirdPersonView.viewMatrix(), camera.cameraPos());

        GLfloat cubeVertices[] = {
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




        std::vector<std::string> faces
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
            glClearDepth(1.0f);

            // ...
            glEnable(GL_DEPTH_TEST);

            camera.updateInput();
            glm::vec3 shiftPos = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::quat meshOrientation;
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


            // Calculate the up vector (usually (0, 1, 0) for a top-down view)
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            // Calculate the right vector (cross product of direction and up)
            glm::vec3 right = glm::normalize(glm::cross(up, direction));

            // Recalculate the up vector (cross product of right and direction)
            up = glm::cross(direction, right);

            // Create the view matrix
            glm::mat4 viewMatrix = glm::lookAt(cameraPosition, carLocation.position, up);
            
            if (moving) {
                if (forward) {
                    carLocation.position.z += 0.2f;
                    topView.changePos(glm::vec3{topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z + 0.2});
                }
                else {
                    carLocation.position.z -= 0.2f;
                    topView.changePos(glm::vec3{ topView.cameraPos().x, topView.cameraPos().y, topView.cameraPos().z - 0.2 });
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

            glm::mat4 skyModelMatrix = glm::translate(glm::mat4(1.0f), camera.cameraPos());
            const glm::mat4 mvpMatrixSky = m_projectionMatrix * view * skyModelMatrix;

            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            
            
            
            glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, 0.1f, 100.0f);
            
            
                m_environmentShader.bind();
                glBindVertexArray(0);
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));
                glUniform3fv(3, 1, glm::value_ptr(camera.cameraPos()));
                glBindVertexArray(cubeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);

                for (GPUMesh& mesh : m_meshes) {
                    if (!kd && !bphong) {
                        m_defaultShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                        if (mesh.hasTextureCoords()) {
                            m_texture1.bind(GL_TEXTURE0);
                            glUniform1i(3, 0);
                            glUniform1i(4, GL_TRUE);
                            glUniform1i(5, GL_FALSE);
                        }
                        else {
                            glUniform1i(4, GL_FALSE);
                            glUniform1i(5, GL_TRUE);
                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));
                        }
                        mesh.draw(m_defaultShader);

                    }
                    else {
                        if (kd) {
                            m_kdShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_kdShader);
                        }
                        else {
                            m_bphongShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_bphongShader);

                        }
                    }

                }

                /* glm::mat4 rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(10.f), glm::vec3(0, 0, 1));
                 glm::mat4 scale1 = glm::scale(glm::mat4(1.0f), glm::vec3(2, 1, 1));
                 glm::mat4 unscale = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));
                 glm::mat4 combined = glm::mat4(1);
                 glm::mat4 translationMatrixArm = glm::translate(glm::mat4(1.0f), glm::vec3{ 12,0,0 });*/

                 // CAN BE USED FOR DAY/NIGHT LIGHTING MAYBE
                 //for (int i = 0; i < 4; i++) {
                 //    for (GPUMesh& mesh : arm) {
                 //        tracker = (tracker + 1) % 9;
                 //        rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(float(arr[tracker])), glm::vec3(0, 0, 1));
                 //        //scale1 = glm::scale(glm::mat4(1.0f), glm::vec3(2, 1, 1));
                 //        //unscale = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));
                 //        
                 //        //translationMatrixArm = glm::translate(glm::mat4(1.0f), glm::vec3{ 0,i + 2,i });
                 //        if (slower != 0) {
                 //            combined = combined;
                 //        }
                 //        
                 //        else if (i >= 1) {
                 //            combined *= translationMatrixArm * rotationS;
                 //        }
                 //        else {
                 //            combined *= translationMatrixArm;
                 //        }
                 //        m_modelMatrix = combined;
                 //        const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                 //        if (!kd && !bphong) {
                 //            m_defaultShader.bind();
                 //            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                 //            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                 //            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                 //            mesh.draw(m_defaultShader);

                 //        }
                 //        else {
                 //            if (kd) {
                 //                m_kdShader.bind();
                 //                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                 //                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                 //                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                 //                glUniform3fv(6, 1, glm::value_ptr(camera.cameraPos()));

                 //                mesh.draw(m_kdShader);
                 //            }
                 //            else {
                 //                m_bphongShader.bind();
                 //                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                 //                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                 //                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                 //                glUniform3fv(6, 1, glm::value_ptr(camera.cameraPos()));

                 //                mesh.draw(m_bphongShader);

                 //            }
                 //        }
                 //    }

                 //    translationMatrixArm = glm::translate(glm::mat4(1.0f), glm::vec3{ 0,(i + 1) * 3,0 });
                 //}


                 //slower = (slower + 1) % 15;



                 ///////////////////////////////////////////////////// START ROBOT ARM //////////////////////////////////////////////////////////////////////////

                glm::mat4 translationMatrixArm1 = glm::translate(glm::mat4(1.0f), glm::vec3{ 15,0,0 });
                for (GPUMesh& mesh : arm) {


                    m_modelMatrix = translationMatrixArm1;
                    const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                    if (!kd && !bphong) {
                        m_defaultShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                        mesh.draw(m_defaultShader);

                    }
                    else {
                        if (kd) {
                            m_kdShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_kdShader);
                        }
                        else {
                            m_bphongShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_bphongShader);

                        }
                    }
                }


                for (int i = 0; i < 3; i++) {
                    for (GPUMesh& mesh : arm) {
                        tracker = (tracker + 1) % 9;
                        rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(float(arr[tracker])), glm::vec3(0, 0, 1));

                        translationMatrixArm1 = translationMatrixArm1 * glm::translate(glm::mat4(1.0f), glm::vec3{ -0.5,4.5,0 }) * rotationS;
                        m_modelMatrix = translationMatrixArm1;
                        const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                        if (!kd && !bphong) {
                            m_defaultShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            mesh.draw(m_defaultShader);

                        }
                        else {
                            if (kd) {
                                m_kdShader.bind();
                                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                                glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                                mesh.draw(m_kdShader);
                            }
                            else {
                                m_bphongShader.bind();
                                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                                glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                                mesh.draw(m_bphongShader);

                            }
                        }
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
                    if (!kd && !bphong) {
                        m_defaultShader.bind();
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                        mesh.draw(m_defaultShader);

                    }
                    else {
                        if (kd) {
                            m_kdShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_kdShader);
                        }
                        else {
                            m_bphongShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                            mesh.draw(m_bphongShader);

                        }
                    }
                }


                for (int i = 0; i < 3; i++) {
                    for (GPUMesh& mesh : arm) {
                        tracker = (tracker + 1) % 9;
                        rotationS = glm::rotate(glm::mat4(1.0f), glm::radians(-float(arr[tracker])), glm::vec3(0, 0, 1));

                        translationMatrixArm2 = translationMatrixArm2 * glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5,4.5,0 }) * rotationS;
                        m_modelMatrix = translationMatrixArm2;
                        const glm::mat4 mvpMatrix = m_projectionMatrix * view * m_modelMatrix;
                        if (!kd && !bphong) {
                            m_defaultShader.bind();
                            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                            mesh.draw(m_defaultShader);

                        }
                        else {
                            if (kd) {
                                m_kdShader.bind();
                                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                                glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                                mesh.draw(m_kdShader);
                            }
                            else {
                                m_bphongShader.bind();
                                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

                                glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                                mesh.draw(m_bphongShader);

                            }
                        }
                    }

                }

                glm::vec4 newLightPos2;
                newLightPos2 = translationMatrixArm2 * glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5,4.5,0 }) * glm::vec4(1);


                Light l2 = Light(newLightPos2, glm::vec3(1), light.returnLightIndex(1).returnAttenuation());
                light.replace(l, 1);



                ///////////////////////////////////////////////// END ROBOT ARM ////////////////////////////////////////////////////////////////////////////////////////






                for (int i = 0; i < 6; i++) {
                    GPUMesh& mesh = skybox[i];
                    m_skyboxShader.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixSky));
                    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(skyModelMatrix));
                    glUniform1i(3, i + 1);
                    glUniform1i(4, GL_TRUE);
                    glUniform1i(5, GL_FALSE);
                    glUniform3fv(6, 1, glm::value_ptr(light.returnLight()[0].returnPos()));

                    mesh.draw(m_skyboxShader);
                }




                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // Render other objects with environment mapping



                terrain.renderTerrain(view, light.returnLight());

 
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
            }

            break;
        case GLFW_KEY_2:
            if (cam1 || top) {
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = false;
                top = false;
            }
            break;

        case GLFW_KEY_3:
            
                /*temp = light_camera;
                light_camera = camera;
                camera = temp;*/
                cam1 = false;
                top = true;

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


    unsigned int loadCubemap(std::vector<std::string> faces);


    std::vector<GPUMesh> m_meshes;
    std::vector<GPUMesh> road;
    std::vector<GPUMesh> skybox;
    std::vector<GPUMesh> arm;
    Texture m_texture1;
    Texture m_texture2;
    Texture m_texture3;
    Texture m_texture4;
    Texture m_texture5;
    Texture m_texture6;
    bool m_useMaterial { true };
    Camera camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera topView{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera light_camera{ &m_window, glm::vec3(1.2f, 1.1f, 0.9f), -glm::vec3(1.2f, 1.1f, 0.9f) };
    Camera temp{ &m_window};
    bool cam1{ true };
    bool top{ false };
    Terrain terrain;
  
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
    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 1.1f, 500.f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };

    int arr[9] = {8, 10, 12, 15, 20 ,15,12,10,8 };
    int tracker = 0;
    int slower = 0;

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


