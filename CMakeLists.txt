cmake_minimum_required(VERSION 3.10)
project(AsiaPark3D_CG VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(AsiaPark3D src/main.cpp)

if(WIN32)
    target_link_libraries(AsiaPark3D PRIVATE OpenGL::GL OpenGL::GLU freeglut)
else()
    target_link_libraries(AsiaPark3D PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT m)
endif()
