#------------------------------------------------------------------------------
# CMakeLists.txt
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)

project(GamagoraGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#------------------------------------------------------------------------------
# Packages
#------------------------------------------------------------------------------
include_directories(external/glad/include external/CImg/include external/tinyply/include)

#------------------------------------------------------------------------------
# - OpenGL
#------------------------------------------------------------------------------
set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(OpenGL REQUIRED)

if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL not found!")
endif()

#------------------------------------------------------------------------------
# - glm
#------------------------------------------------------------------------------
find_package(GLM REQUIRED)

if(NOT GLM_FOUND)
	message(FATAL_ERROR "glm not found!")
endif(NOT GLM_FOUND)

include_directories(${GLM_INCLUDE_DIRS})

#------------------------------------------------------------------------------
# - glfw
#------------------------------------------------------------------------------
find_package(GLFW REQUIRED)

if(NOT GLFW_FOUND)
	message(FATAL_ERROR "glfw not found!")
endif()

include_directories(${GLFW_INCLUDE_DIR})

#------------------------------------------------------------------------------
# - X11
#------------------------------------------------------------------------------
find_package(X11 REQUIRED)

if(NOT X11_FOUND)
	message(FATAL_ERROR "X11 not found!")
endif()

include_directories(${X11_INCLUDE_DIR})

#------------------------------------------------------------------------------
# - Threads
#------------------------------------------------------------------------------
find_package(Threads REQUIRED)

if(THREADS_HAVE_PTHREAD_ARG)
	target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-pthread")
endif()

#------------------------------------------------------------------------------
# Get source files
#------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCE_FILES source/*.cpp external/glad/src/glad.c)
file(GLOB_RECURSE HEADER_FILES source/*.hpp source/*.h)

foreach(HEADER_FILE ${HEADER_FILES})
	get_filename_component(HEADER_DIRECTORY ${HEADER_FILE} DIRECTORY)
	include_directories(${HEADER_DIRECTORY})
endforeach(HEADER_FILE)

#------------------------------------------------------------------------------
# Add executables
#------------------------------------------------------------------------------
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

#------------------------------------------------------------------------------
# Link options
#------------------------------------------------------------------------------
target_link_libraries(${CMAKE_PROJECT_NAME}
                      ${CMAKE_DL_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${X11_LIBRARIES}
                      ${GLFW_LIBRARY}
                      ${OPENGL_LIBRARIES})

