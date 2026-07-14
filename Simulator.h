#pragma once

#include "rtre.h"
#include "GLFW/rtre_window.h"
#include <vector>
#include <memory>
#include <string>

enum class ShapeType {
    eNONE = -1,
    eSphere,
    eBox,
    eTriangle
};

struct Hitdata {
    float distance;
    uint32_t index;
    ShapeType type;
};

class Simulator {
public:
    Simulator();

    ~Simulator();

    void init(rtre::Window &window);

    void run();

private:
    void handleInput();

    void update();

    void render();

    void renderUI();

    void resetAccumulation();

    void addRandomSphere();

    void addRandomBox();

    void addRandomTriangle();

    static Hitdata mymin(Hitdata p1, Hitdata p2);

    static bool sphereMenu(rtre::Sphere &sphere);

    static bool boxMenu(rtre::Box &box);

    static bool triangleMenu(rtre::Triangle &triangle);

    static glm::mat4 getCameraMatrix(const rtre::Camera &c);

    static float my_random();

private:
    rtre::Window *m_Window = nullptr;

    std::shared_ptr<rtre::RenderShader> m_PathTraceShader;
    std::shared_ptr<rtre::RenderShader> m_DisplayShader;

    std::unique_ptr<rtre::Quad> m_PathTraceQuad;
    std::unique_ptr<rtre::Quad> m_DisplayQuad;

    GLuint m_PathTracerFBO = 0;
    GLuint m_PathTracerTextures[2] = {0, 0};
    int m_CurrentTexture = 0;

    float m_FrameCounter = 1.0f;
    uint32_t m_LastWidth = 0;
    uint32_t m_LastHeight = 0;

    bool m_Reset = false;
    glm::vec3 m_LastCamPos;
    glm::vec3 m_LastCamOr;

    Hitdata m_SelectedShape = {-1.0f, 0xffffffff, ShapeType::eNONE};

    float m_Speed = 20.0f; // Multiplied by some factor in original code
    int m_Bounces = 2;
    float m_MyTime = 0.0f;
    double m_StartTime = 0.0;

    rtre::Ubo m_SceneUBO;
    rtre::Ssbo m_SphereSSBO;
    rtre::Ssbo m_BoxSSBO;
    rtre::Ssbo m_TriangleSSBO;

    bool m_SpheresDirty = true;
    bool m_BoxesDirty = true;
    bool m_TrianglesDirty = true;

    bool m_ShowControlPanel = true;
    bool m_ShowObjectList = true;
};