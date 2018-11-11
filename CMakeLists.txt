cmake_minimum_required(VERSION 3.1)
project(elastic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(JSON_HPP_SRC_DIR "/home/vismay/recode/json/single_include/nlohmann")
set(SPECTRA_INCLUDE_DIR "/home/vismay/spectra-0.5.0/include")
set(SUITESPARSE_HINT "/home/vismay/recode/SuiteSparse")
set(ALGLIB_SRC_DIR "/home/vismay/recode/alglib-cpp/src")

# libigl
option(LIBIGL_USE_STATIC_LIBRARY "Use libigl as static library" OFF)
option(LIBIGL_WITH_ANTTWEAKBAR      "Use AntTweakBar"    OFF)
option(LIBIGL_WITH_CGAL             "Use CGAL"           OFF)
option(LIBIGL_WITH_COMISO           "Use CoMiso"         OFF)
option(LIBIGL_WITH_CORK             "Use Cork"           OFF)
option(LIBIGL_WITH_EMBREE           "Use Embree"         OFF)
option(LIBIGL_WITH_LIM              "Use LIM"            OFF)
option(LIBIGL_WITH_MATLAB           "Use Matlab"         OFF)
option(LIBIGL_WITH_MOSEK            "Use MOSEK"          OFF)
option(LIBIGL_WITH_NANOGUI          "Use Nanogui menu"   OFF)
option(LIBIGL_WITH_OPENGL           "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW      "Use GLFW"           ON)
option(LIBIGL_WITH_PNG              "Use PNG"            OFF)
option(LIBIGL_WITH_PYTHON           "Use Python"         OFF)
option(LIBIGL_WITH_TETGEN           "Use Tetgen"         ON)
option(LIBIGL_WITH_TRIANGLE         "Use Triangle"       OFF)
option(LIBIGL_WITH_VIEWER           "Use OpenGL viewer"  ON)
option(LIBIGL_WITH_XML              "Use XML"            OFF)

find_package(LIBIGL REQUIRED QUIET)
find_package(SUITESPARSE REQUIRED QUIET)

include_directories(${JSON_HPP_SRC_DIR})
include_directories(${SPECTRA_INCLUDE_DIR})
include_directories(${SUITESPARSE_INCLUDE_DIRS})
include_directories(${ALGLIB_SRC_DIR})

# Add your project files
file(GLOB HEADFILES "*.h" "${ALGLIB_SRC_DIR}/*.h")
file(GLOB SRCFILES "${ALGLIB_SRC_DIR}/*.cpp")
#file(GLOB EXTERNALS "../spectra-0.5.0/include/*.h")
add_executable(${PROJECT_NAME} "main.cpp" ${HEADFILES} ${SRCFILES} ${EXTERNALS})
#add_executable(arap_ex "arap_main.cpp" ${HEADFILES} ${SRCFILES} ${EXTERNALS})
target_link_libraries(${PROJECT_NAME} -lglfw igl::core igl::opengl igl::opengl_glfw -fopenmp ${SUITESPARSE_LIBRARIES})
#target_link_libraries(arap_ex -lglfw igl::core igl::opengl igl::opengl_glfw -fopenmp ${SUITESPARSE_LIBRARIES})