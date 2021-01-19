#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Light {
public:
	glm::vec3 position;
	glm::vec3 color;
	float intensity;

	Light(glm::vec3 pos, glm::vec3 c, float intensity);
};