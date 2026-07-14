cmake_minimum_required(VERSION 3.10)
project(Raytrace)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(OpenGL REQUIRED)
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
    find_package(glfw REQUIRED)
endif ()
find_package(glm CONFIG REQUIRED)

include_directories(
        .
        include
        implimgui
        engine_abstractions
        engine_meshes
        engine_movement
        engine_rendering
)

set(SOURCES
        Main.cpp
        Simulator.cpp
        engine_abstractions/dependencies/glad.c
        engine_abstractions/dependencies/stb_image.cpp
        engine_abstractions/dependencies/stb_image_write.cpp
        implimgui/imgui.cpp
        implimgui/imgui_demo.cpp
        implimgui/imgui_draw.cpp
        implimgui/imgui_impl_glfw.cpp
        implimgui/imgui_impl_opengl3.cpp
        implimgui/imgui_tables.cpp
        implimgui/imgui_widgets.cpp
        include/GLFW/rtre_Window.cpp
)

add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

add_executable(Raytrace ${SOURCES})

if (TARGET glfw)
    target_link_libraries(Raytrace glfw)
elseif (TARGET glfw3)
    target_link_libraries(Raytrace glfw3)
else ()
    target_link_libraries(Raytrace ${GLFW_LIBRARIES})
endif ()

target_link_libraries(Raytrace
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
        glm::glm
)