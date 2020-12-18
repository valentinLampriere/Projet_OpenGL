#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include "shader.h"

#define TINYPLY_IMPLEMENTATION
//#include <tinyply.h>

#include "stl.h"
#include "texture.h"

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

int main(void) {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	// Initialise GLFW
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(1024, 768, "Tutorials", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window); // Initialise GLEW
	glfwSwapInterval(1);

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "resources/shaders/shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "resources/shaders/shader.frag");

	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	static const GLfloat g_triangleVertex_buffer_data[] = {
		-2.0f, -2.0f, 0.0f,
		0.0f, 2.0f, 0.0f,
		2.0f, -2.0f, 0.0f
	};

	static const GLfloat g_cubeVertex_buffer_data[] = {
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
	};

	// One color for each vertex. They were generated randomly.
	static GLfloat g_color_buffer_data[12 * 3 * 3];
	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> dist(0.0f, 1.0f);
	for (int v = 0; v < 12 * 3; v++) {
		g_color_buffer_data[3 * v + 0] = dist(e2);
		g_color_buffer_data[3 * v + 1] = dist(e2);
		g_color_buffer_data[3 * v + 2] = dist(e2);
	}

	// Buffers //
	GLuint cubeVertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &cubeVertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_cubeVertex_buffer_data), g_cubeVertex_buffer_data, GL_STATIC_DRAW);

	GLuint triangleVertexbuffer;
	glGenBuffers(1, &triangleVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangleVertex_buffer_data), g_triangleVertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	GLuint MatrixID = glGetUniformLocation(program, "MVP");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		float u_time = glfwGetTime();
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		
		// Camera matrix
		glm::mat4 View = glm::lookAt(
			glm::vec3(4, 3, -3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);

		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 mvp = Projection * View * Model;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 6*2*3 + 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
