
#include "controls.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// position
glm::vec3 position = glm::vec3(0, 0, 5);
// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 4.0f;
float verticalSpeed = 2.0f;
float mouseSpeed = 0.015f;

float lastTime = 0;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

glm::vec3 getOrbitPos(glm::vec3 origin, glm::vec3 distance, float speed, float iTime) {
	return glm::vec3(cos(iTime * speed) * distance.x + origin.x, sin(iTime * speed) * distance.y + origin.y, sin(iTime * speed) * distance.z + origin.z);
}

void computeMatricesFromInputs(GLFWwindow* window) {
	float currentTime = (float) glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	// Get mouse position
	double xpos, ypos;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, width / 2, height / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * deltaTime * float(width / 2 - xpos);
	verticalAngle += mouseSpeed * deltaTime * float(height / 2 - ypos);

	if (verticalAngle < -1) {
		verticalAngle = -1;
	} else if (verticalAngle > 1) {
		verticalAngle = 1;
	}


	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);
	glm::vec3 up = glm::cross(right, direction);


	/*glm::vec3 origin = glm::vec3(0, 0, 0);
	position = getOrbitPos(origin, glm::vec3(10, 0, 10), 0.5, currentTime);
	direction = origin - position;*/
		

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		position += glm::vec3(0, 1, 0) * deltaTime * verticalSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		position -= glm::vec3(0, 1, 0) * deltaTime * verticalSpeed;
	}

	float FoV = initialFoV;

	// Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	lastTime = currentTime;
}

