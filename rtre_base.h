#pragma once
#include <memory>
#include <iostream>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <vector>
#include <array>
#include <string>

#include "engine_abstractions/Shader.h"
#include "engine_abstractions/Sampler.h"
#include "engine_abstractions/dtypes.h"
#include "engine_rendering/camera.h"
#include "GLFW/rtre_window.h"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif

namespace rtre {
    inline Camera camera;
    inline GLuint viewportWidth;
    inline GLuint viewportHeight;
    inline Window *eWindow;
    inline GLfloat aspectRatio = 1;
    inline Sampler3D *skyBox = nullptr;
    inline const GLuint skyboxUnit = 0;
    inline std::vector<rtre::Sphere *> sphereList;
    inline std::vector<rtre::Box *> boxList;
    inline std::vector<rtre::Triangle *> triangleList;

    inline std::array<std::string, 6> cubemap = {
        PROJECT_ROOT "skybox/right.png",
        PROJECT_ROOT "skybox/left.png",
        PROJECT_ROOT "skybox/up.png",
        PROJECT_ROOT "skybox/down.png",
        PROJECT_ROOT "skybox/forward.png",
        PROJECT_ROOT "skybox/back.png"
    };

    inline void setViewport(GLuint vWidth, GLuint vHeight)
    {
        viewportWidth = vWidth;
        viewportHeight = vHeight;
        aspectRatio = viewportWidth / (GLfloat) viewportHeight;

        camera.setAspectRatio(aspectRatio);

        glViewport(0, 0, viewportWidth, viewportHeight);
    }

    /*
	Initilize glad
	Must be called after setting window context
	*/
    inline void init(GLuint viewportWidth, GLuint viewportHeight, Window &window,
                     const glm::vec3 &pos = glm::vec3(5, 5, 5), GLfloat aspectRatio = 1.0f,
                     GLfloat fov = 75.0f, GLfloat zNear = 0.05f, GLfloat zFar = 500.0f)
    {
        if (!gladLoadGL())
        {
            throw std::runtime_error("Could not load glad.\n");
        }

        setViewport(viewportWidth, viewportHeight);

        skyBox = new Sampler3D(cubemap, skyboxUnit);

        camera = Camera(pos, aspectRatio, fov, zNear, zFar);

        eWindow = &window;
    }


    inline void enable(int glflags)
    {
        glEnable(glflags);
    }

    inline void cullFace(int glflag)
    {
        glCullFace(glflag);
    }

    inline void setBackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 0.0)
    {
        glClearColor(r, g, b, a);
    }

    inline void setBackgroundColor(vec3 vec, GLfloat a = 0.0)
    {
        glClearColor(vec.x, vec.y, vec.z, a);
    }

    inline void setBackgroundColor(vec4 vec)
    {
        glClearColor(vec.x, vec.y, vec.z, vec.y);
    }

    inline void clearBuffers(int glflags)
    {
        glClear(glflags);
    }

    /*
	Set which faces to be considered the front ones
	Decision is made based on the indices ordering/direction
	Arguments are either, GL_CCW or GL_CW (clockwise/counter clockwise)
	*/
    inline void setFrontFace(int glflag)
    {
        glFrontFace(glflag);
    }
}