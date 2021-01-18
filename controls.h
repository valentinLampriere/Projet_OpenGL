#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 getViewMatrix();

glm::mat4 getProjectionMatrix();

void computeMatricesFromInputs(GLFWwindow* window);