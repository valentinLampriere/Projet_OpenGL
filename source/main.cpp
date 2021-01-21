#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include "shader.h"
#include "../vbo_indexer.h"

#define TINYPLY_IMPLEMENTATION
//#include <tinyply.h>

#include "stl.h"
#include "../Light.h"
#include "texture.h"
#include "../controls.h"

using namespace std;

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



GLuint loadBMP_custom(const char* path) {
	
	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	// Actual RGB data
	unsigned char* data;

	// Open the file
	FILE* file = fopen(path, "rb");
	if (!file) { printf("Image could not be opened\n"); return 0; }

	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return 0;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	//Everything is in memory now, the file can be closed
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Generate mipmaps, by the way.
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

bool loadOBJ(char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals) {
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		} else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		} else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		} else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		} else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}


int main(void) {

	int width = 1024;
	int height = 768;

	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	// Initialise GLFW
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(width, height, "Tutorials", NULL, NULL);

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


	GLuint Texture = loadBMP_custom("./img/uvtemplate.bmp");
	GLuint TextureID = glGetUniformLocation(program, "cubeTexture");

	std::vector<glm::vec3> cube_vertices;
	std::vector<glm::vec2> cube_uvs;
	std::vector<glm::vec3> cube_normals; // Won't be used at the moment.
	if (!loadOBJ("resources/models/cube.obj", cube_vertices, cube_uvs, cube_normals)) {
		return -1;
	}
	for (int i = 0; i < cube_vertices.size(); i++) {
		cube_vertices[i] += glm::vec3(0, -2, 0);
	}

	// Buffers //
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

#pragma region monkey buffers

	std::vector<glm::vec3> inmonkey_vertices;
	std::vector<glm::vec2> inmonkey_uvs;
	std::vector<glm::vec3> inmonkey_normals;

	std::vector<glm::vec3> monkey_vertices;
	std::vector<glm::vec2> monkey_uvs;
	std::vector<glm::vec3> monkey_normals;

	std::vector<unsigned short> indicesMonkey;

	if (!loadOBJ("resources/models/monkey.obj", inmonkey_vertices, inmonkey_uvs, inmonkey_normals)) {
		std::cout << "Can't load monkey :(";
		return -1;
	}

	indexVBO(inmonkey_vertices, inmonkey_uvs, inmonkey_normals, indicesMonkey, monkey_vertices, monkey_uvs, monkey_normals);

	GLuint monkey_vertexbuffer;
	glGenBuffers(1, &monkey_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkey_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkey_vertices.size() * sizeof(glm::vec3), &monkey_vertices[0], GL_STATIC_DRAW);

	GLuint monkey_uvbuffer;
	glGenBuffers(1, &monkey_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkey_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkey_uvs.size() * sizeof(glm::vec2), &monkey_uvs[0], GL_STATIC_DRAW);

	GLuint monkey_normalbuffer;
	glGenBuffers(1, &monkey_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkey_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkey_normals.size() * sizeof(glm::vec3), &monkey_normals[0], GL_STATIC_DRAW);

	GLuint elementbufferMonkey;
	glGenBuffers(1, &elementbufferMonkey);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferMonkey);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesMonkey.size() * sizeof(unsigned short), &indicesMonkey[0], GL_STATIC_DRAW);

#pragma endregion
#pragma region cube buffers
	GLuint cube_vertexbuffer;
	glGenBuffers(1, &cube_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(glm::vec3), &cube_vertices[0], GL_STATIC_DRAW);

	GLuint cube_uvbuffer;
	glGenBuffers(1, &cube_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_uvs.size() * sizeof(glm::vec2), &cube_uvs[0], GL_STATIC_DRAW);

	GLuint cube_normalbuffer;
	glGenBuffers(1, &cube_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_normals.size() * sizeof(glm::vec3), &cube_normals[0], GL_STATIC_DRAW);
#pragma endregion

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Enable transparencies
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glClearColor(0.2, 0.2, 0.3, 0);

	GLuint MatrixID = glGetUniformLocation(program, "MVP");

	GLuint ModelMatrixID = glGetUniformLocation(program, "M");
	GLuint ViewMatrixID = glGetUniformLocation(program, "V");

	Light light = Light(glm::vec3(2, 10, 5), glm::vec3(0.9, 0.9, 0.8), 200);
	GLuint LightPositionID = glGetUniformLocation(program, "lightPosition");
	GLuint LightColorID = glGetUniformLocation(program, "lightColor");
	GLuint LightIntensityID = glGetUniformLocation(program, "lightIntensity");


	glfwSetCursorPos(window, width / 2, height / 2);
	
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (!glfwWindowShouldClose(window)) {
		float u_time = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

#pragma region monkey
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, monkey_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, monkey_uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, monkey_normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, monkey_vertices.size() * sizeof(glm::vec3));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferMonkey);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indicesMonkey.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);
#pragma endregion

#pragma region cube
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cube_uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, cube_normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size() * sizeof(glm::vec3));
#pragma endregion

		computeMatricesFromInputs(window);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;




		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniform3f(LightPositionID, light.position.x, light.position.y, light.position.z);
		glUniform3f(LightColorID, light.color.r, light.color.g, light.color.b);
		glUniform1f(LightIntensityID, light.intensity);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
