#include "Simulator.h"
#include "engine_movement/controller.h"
#include <ctime>
#include <algorithm>

// Static callback for 'F' key to toggle cursor mode
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
        if (cursorMode == GLFW_CURSOR_DISABLED)
            cursorMode = GLFW_CURSOR_NORMAL;
        else
            cursorMode = GLFW_CURSOR_DISABLED;

        glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
    }
}

Simulator::Simulator()
{
}

Simulator::~Simulator()
{
    if (m_PathTracerFBO)
        glDeleteFramebuffers(1, &m_PathTracerFBO);
    if (m_PathTracerTextures[0])
        glDeleteTextures(2, m_PathTracerTextures);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Simulator::init(rtre::Window &window)
{
    m_Window = &window;

    // Initialize RTRE core
    rtre::init(window.getWindowSize().width, window.getWindowSize().height, window, glm::vec3(5, 5, 5));

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // Load Shaders
    m_PathTraceShader = std::make_shared<rtre::RenderShader>(
        PROJECT_ROOT "engine_resources/vert.vert", PROJECT_ROOT "engine_resources/frag.frag", "");
    m_PathTraceQuad = std::make_unique<rtre::Quad>(m_PathTraceShader);

    m_DisplayShader = std::make_shared<rtre::RenderShader>(
        PROJECT_ROOT "engine_resources/main.vert", PROJECT_ROOT "engine_resources/main.frag", "");
    m_DisplayQuad = std::make_unique<rtre::Quad>(m_DisplayShader);

    // Setup FBO
    glGenFramebuffers(1, &m_PathTracerFBO);

    // Initial scene
    srand(time(0));
    for (int i = 0; i < 10; i++) addRandomSphere();
    for (int i = 0; i < 10; i++) addRandomBox();

    m_StartTime = glfwGetTime();

    glfwSetKeyCallback(window.getWindow(), key_callback);
}

void Simulator::addRandomSphere()
{
    rtre::sphereList.emplace_back(
        new rtre::Sphere(
            glm::vec3(my_random() * 10, my_random() * 10, my_random() * 10),
            my_random() * 2,
            rtre::Material(
                glm::vec3(my_random(), my_random(), my_random()),
                my_random(),
                my_random(),
                0.0f,
                my_random()
            )
        )
    );
    m_SpheresDirty = true;
    resetAccumulation();
}

void Simulator::addRandomBox()
{
    rtre::boxList.emplace_back(
        new rtre::Box(
            glm::vec3(my_random() * 10, my_random() * 10, my_random() * 10),
            glm::vec3(my_random() * 2, my_random() * 2, my_random() * 2),
            rtre::Material(
                glm::vec3(my_random(), my_random(), my_random()),
                my_random(),
                my_random(),
                0.0f,
                my_random()
            )
        )
    );
    m_BoxesDirty = true;
    resetAccumulation();
}

void Simulator::resetAccumulation()
{
    m_Reset = true;
    m_FrameCounter = 1.0f;
}

void Simulator::run()
{
    while (!m_Window->shouldClose() && !m_Window->isKeyPressed(GLFW_KEY_ESCAPE))
    {
        update();
        render();
    }
}

void Simulator::update()
{
    rtre::Window::pollEvents();

    m_PathTraceShader->checkAndHotplug();
    m_DisplayShader->checkAndHotplug();

    int display_w, display_h;
    glfwGetFramebufferSize(m_Window->getWindow(), &display_w, &display_h);

    if (m_LastWidth != (uint32_t) display_w || m_LastHeight != (uint32_t) display_h)
    {
        m_LastWidth = display_w;
        m_LastHeight = display_h;

        glBindFramebuffer(GL_FRAMEBUFFER, m_PathTracerFBO);
        if (m_PathTracerTextures[0] != 0)
            glDeleteTextures(2, m_PathTracerTextures);

        glGenTextures(2, m_PathTracerTextures);
        for (int i = 0; i < 2; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, m_PathTracerTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, display_w, display_h, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glViewport(0, 0, display_w, display_h);
        resetAccumulation();
    }

    handleInput();

    m_MyTime = (float) (glfwGetTime() - m_StartTime);
    rtre::camera.setSpeed(glm::vec3(m_Speed));
    rtre::controller::control();
}

void Simulator::handleInput()
{
    if (glm::length(m_LastCamPos - rtre::camera.position()) > 0.0001f ||
        glm::length(m_LastCamOr - rtre::camera.orientation()) > 0.0001f)
    {
        resetAccumulation();
        m_LastCamPos = rtre::camera.position();
        m_LastCamOr = rtre::camera.orientation();
    }

    if (m_Window->isClickingLeft() && !ImGui::GetIO().WantCaptureMouse)
    {
        int w, h;
        glfwGetFramebufferSize(m_Window->getWindow(), &w, &h);
        float aspect = (float) w / h;

        float xCoord = (m_Window->getCursorPosition().x / (float) w) * 2.0f - 1.0f;
        float yCoord = (m_Window->getCursorPosition().y / (float) h) * 2.0f - 1.0f;

        glm::mat4 pmatrix = getCameraMatrix(rtre::camera);
        glm::vec2 coord = glm::vec2(xCoord * aspect, -yCoord) / 2.0f;
        glm::vec4 rd = pmatrix * glm::vec4(glm::normalize(glm::vec3(coord, -1.0f)), 0.0f);
        glm::vec3 rayDirection = glm::vec3(rd);
        rtre::Ray ray = rtre::Ray(rayDirection, rtre::camera.position());

        Hitdata data = {-1.0f, 0xffffffff, ShapeType::eNONE};
        for (uint32_t i = 0; i < rtre::sphereList.size(); i++)
        {
            data = mymin(data, Hitdata{rtre::sphereList[i]->intersect(ray), i, ShapeType::eSphere});
        }
        for (uint32_t i = 0; i < rtre::boxList.size(); i++)
        {
            data = mymin(data, Hitdata{rtre::boxList[i]->intersect(ray), i, ShapeType::eBox});
        }

        if (data.distance > 0.0)
        {
            m_SelectedShape = data;
        }
    }
}

void Simulator::render()
{
    int w, h;
    glfwGetFramebufferSize(m_Window->getWindow(), &w, &h);
    float aspect = (float) w / h;
    glm::mat4 pmatrix = getCameraMatrix(rtre::camera);

    // Update Scene UBO
    rtre::SceneData sceneData;
    sceneData.viewInverse = pmatrix;
    sceneData.cameraPos = glm::vec4(rtre::camera.position(), 1.0f);
    sceneData.time = m_MyTime;
    sceneData.aspectRatio = aspect;
    sceneData.frameCounter = m_FrameCounter;
    sceneData.bounces = m_Bounces;
    sceneData.sphereNum = (uint32_t) rtre::sphereList.size();
    sceneData.boxNum = (uint32_t) rtre::boxList.size();
    sceneData.state = (uint32_t) rand();
    sceneData.reset = m_Reset ? 1 : 0;
    m_SceneUBO.loadData(sceneData);
    m_SceneUBO.bindBase(0);

    // Update Sphere SSBO if dirty
    if (m_SpheresDirty)
    {
        std::vector<rtre::GPUSphere> gpuSpheres;
        for (auto s: rtre::sphereList)
        {
            rtre::GPUSphere gs;
            gs.position = s->position;
            gs.radius = s->radius;
            gs.material.albedo = s->material.albedo;
            gs.material.roughness = s->material.roughness;
            gs.material.metalic = s->material.metalic;
            gs.material.emissive = s->material.emissive;
            gs.material.specular = s->material.specular;
            gpuSpheres.push_back(gs);
        }
        m_SphereSSBO.loadData(gpuSpheres);
        m_SpheresDirty = false;
    }
    m_SphereSSBO.bindBase(1);

    // Update Box SSBO if dirty
    if (m_BoxesDirty)
    {
        std::vector<rtre::GPUBox> gpuBoxes;
        for (auto b: rtre::boxList)
        {
            rtre::GPUBox gb;
            gb.position = b->position;
            gb.dimensions = b->dimensions;
            gb.material.albedo = b->material.albedo;
            gb.material.roughness = b->material.roughness;
            gb.material.metalic = b->material.metalic;
            gb.material.emissive = b->material.emissive;
            gb.material.specular = b->material.specular;
            gpuBoxes.push_back(gb);
        }
        m_BoxSSBO.loadData(gpuBoxes);
        m_BoxesDirty = false;
    }
    m_BoxSSBO.bindBase(2);

    // 1. Path Tracing Pass
    m_CurrentTexture = 1 - m_CurrentTexture;
    glBindFramebuffer(GL_FRAMEBUFFER, m_PathTracerFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PathTracerTextures[m_CurrentTexture],
                           0);

    m_PathTraceShader->activate();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_PathTracerTextures[1 - m_CurrentTexture]);
    m_PathTraceShader->SetUniform("text", 1);
    m_PathTraceShader->SetUniform("cubemap", (GLint) rtre::skyBox->unit());

    rtre::skyBox->bind();
    m_PathTraceQuad->draw();

    // 2. Display Pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_DisplayShader->activate();
    m_DisplayShader->SetUniform("aspec", aspect);
    m_DisplayShader->SetUniform("current", (GLint) 0);
    m_DisplayShader->SetUniform("prev", (GLint) 1);
    m_DisplayShader->SetUniform("counter", m_FrameCounter);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_PathTracerTextures[m_CurrentTexture]);

    m_DisplayQuad->draw();

    // 3. UI Pass
    renderUI();

    m_Window->swapBuffers();

    m_Reset = false;
    m_FrameCounter += 0.5f;
}

void Simulator::renderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_ShowControlPanel)
    {
        ImGui::Begin("Control Panel", &m_ShowControlPanel);

        if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Camera Speed", &m_Speed, 1.0f, 200.0f, "%.1f");

            if (ImGui::SliderInt("Max Bounces", &m_Bounces, 0, 50))
            {
                resetAccumulation();
            }

            if (ImGui::Button("Reset Camera"))
            {
                rtre::camera.setPosition(glm::vec3(1));
                rtre::camera.setOrientation(glm::vec3(0, 0, -1));
                resetAccumulation();
            }
        }

        if (ImGui::CollapsingHeader("Object Management", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Add Random Sphere")) addRandomSphere();
            ImGui::SameLine();
            if (ImGui::Button("Add Random Box")) addRandomBox();

            if (ImGui::TreeNode("Spheres"))
            {
                for (uint32_t i = 0; i < rtre::sphereList.size(); i++)
                {
                    std::string label = "Sphere " + std::to_string(i);
                    if (ImGui::Selectable(label.c_str(),
                                          m_SelectedShape.type == ShapeType::eSphere && m_SelectedShape.index == i))
                    {
                        m_SelectedShape = {1.0f, i, ShapeType::eSphere};
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Boxes"))
            {
                for (uint32_t i = 0; i < rtre::boxList.size(); i++)
                {
                    std::string label = "Box " + std::to_string(i);
                    if (ImGui::Selectable(label.c_str(),
                                          m_SelectedShape.type == ShapeType::eBox && m_SelectedShape.index == i))
                    {
                        m_SelectedShape = {1.0f, i, ShapeType::eBox};
                    }
                }
                ImGui::TreePop();
            }
        }

        if (m_SelectedShape.distance > 0.0)
        {
            if (ImGui::CollapsingHeader("Selected Object", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (m_SelectedShape.type == ShapeType::eSphere && m_SelectedShape.index < rtre::sphereList.size())
                {
                    if (sphereMenu(*rtre::sphereList[m_SelectedShape.index]))
                    {
                        m_SpheresDirty = true;
                        resetAccumulation();
                    }
                } else if (m_SelectedShape.type == ShapeType::eBox && m_SelectedShape.index < rtre::boxList.size())
                {
                    if (boxMenu(*rtre::boxList[m_SelectedShape.index]))
                    {
                        m_BoxesDirty = true;
                        resetAccumulation();
                    }
                }

                if (ImGui::Button("Delete Object"))
                {
                    if (m_SelectedShape.type == ShapeType::eSphere)
                    {
                        delete rtre::sphereList[m_SelectedShape.index];
                        rtre::sphereList.erase(rtre::sphereList.begin() + m_SelectedShape.index);
                        m_SpheresDirty = true;
                    } else if (m_SelectedShape.type == ShapeType::eBox)
                    {
                        delete rtre::boxList[m_SelectedShape.index];
                        rtre::boxList.erase(rtre::boxList.begin() + m_SelectedShape.index);
                        m_BoxesDirty = true;
                    }
                    m_SelectedShape = {-1.0f, 0xffffffff, ShapeType::eNONE};
                    resetAccumulation();
                }
            }
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Hitdata Simulator::mymin(Hitdata p1, Hitdata p2)
{
    if (p1.distance < 0.0) return p2;
    if (p2.distance < 0.0) return p1;
    return (p1.distance < p2.distance) ? p1 : p2;
}

bool Simulator::sphereMenu(rtre::Sphere &sphere)
{
    bool changed = false;
    changed |= ImGui::SliderFloat("Radius", &sphere.radius, 0.1f, 10.0f);
    changed |= ImGui::DragFloat3("Position", (float *) &sphere.position, 0.1f);
    ImGui::Separator();
    changed |= ImGui::ColorEdit3("Albedo", (float *) &sphere.material.albedo);
    changed |= ImGui::SliderFloat("Roughness", &sphere.material.roughness, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metalic", &sphere.material.metalic, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Emissive", &sphere.material.emissive, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Specular", &sphere.material.specular, 0.0f, 1.0f);
    return changed;
}

bool Simulator::boxMenu(rtre::Box &box)
{
    bool changed = false;
    changed |= ImGui::DragFloat3("Position", (float *) &box.position, 0.1f);
    changed |= ImGui::DragFloat3("Dimensions", (float *) &box.dimensions, 0.1f, 0.1f, 10.0f);
    ImGui::Separator();
    changed |= ImGui::ColorEdit3("Albedo", (float *) &box.material.albedo);
    changed |= ImGui::SliderFloat("Roughness", &box.material.roughness, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metalic", &box.material.metalic, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Emissive", &box.material.emissive, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Specular", &box.material.specular, 0.0f, 1.0f);
    return changed;
}

glm::mat4 Simulator::getCameraMatrix(const rtre::Camera &c)
{
    auto view = glm::lookAt(c.position(), c.position() + c.orientation(), glm::vec3(0, 1, 0));
    return glm::inverse(view);
}

float Simulator::my_random()
{
    return (float) rand() / (float) RAND_MAX;
}
