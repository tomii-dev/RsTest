cmake_minimum_required(VERSION 3.6)

# this file should not be run manually, please use build.py
# and pass the path to where you extracted the zip from https://glew.sourceforge.net/
# as a command line argument

# however if you do really want to run this manually,
# set the cached var GLEW_DIR to the extracted zip path
# e.g. cmake ./ -DGLEW_DIR=C:/Some/Path/glew-2.1.0

project(RsTest)

find_package(OpenGL REQUIRED)

# git submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/app.cpp
    src/camera.cpp
    src/lightsource.cpp
    src/object.cpp
    src/scene.cpp
    src/shape.cpp
    src/utils.cpp

    # imgui src files
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/misc/cpp/imgui_stdlib.cpp
)

# build rules for deps
add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC external/glfw/include
    PUBLIC ${GLEW_DIR}/include
    PUBLIC external/glm
    PUBLIC external/imgui
)

target_link_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/external/glfw/src
    PRIVATE ${GLEW_DIR}/lib/Release/x64
)

target_link_libraries(${PROJECT_NAME} glfw3 glew32s shlwapi ${OPENGL_LIBRARIES})