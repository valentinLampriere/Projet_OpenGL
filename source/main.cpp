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


#define TINYPLY_IMPLEMENTATION
//#include <tinyply.h>

#include "stl.h"
#include "../Light.h"
#include "texture.h"
#include "../controls.h"
#include "../Mesh.h"

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



void computeTangentBasis(
	// inputs
	std::vector<glm::vec3>& vertices,
	std::vector<glm::vec2>& uvs,
	std::vector<glm::vec3>& normals,
	// outputs
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec3>& bitangents
) {
	for (int i = 0; i < vertices.size(); i += 3) {
		// Shortcuts for vertices
		glm::vec3& v0 = vertices[i + 0];
		glm::vec3& v1 = vertices[i + 1];
		glm::vec3& v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2& uv0 = uvs[i + 0];
		glm::vec2& uv1 = uvs[i + 1];
		glm::vec2& uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		// Set the same tangent for all three vertices of the triangle.
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}
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

	// Buffers //
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

#pragma region lego buffers

	/*std::vector<glm::vec3> inlego_vertices;
	std::vector<glm::vec2> inlego_uvs;
	std::vector<glm::vec3> inlego_normals;

	std::vector<glm::vec3> lego_vertices;
	std::vector<glm::vec2> lego_uvs;
	std::vector<glm::vec3> lego_normals;

	std::vector<glm::vec3> lego_color;

	std::vector<unsigned short> indicesLego;
	*/
	Mesh legoMesh = Mesh();
	legoMesh.loadMesh("resources/models/lego2.obj", &legoMesh, false);
	//legoMesh.setColor(glm::vec3(0, 0, 1));

	/*
	if (!loadOBJ("resources/models/lego2.obj", inlego_vertices, inlego_uvs, inlego_normals)) {
		std::cout << "Can't load lego :(";
		return -1;
	}

	indexVBO(inlego_vertices, inlego_uvs, inlego_normals, indicesLego, lego_vertices, lego_uvs, lego_normals);

	for (int i = 0; i < lego_vertices.size(); i++) {
		lego_color.push_back(glm::vec3(0.7, 0.5, 0.1));
	}

	GLuint lego_vertexbuffer;
	glGenBuffers(1, &lego_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lego_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, lego_vertices.size() * sizeof(glm::vec3), &lego_vertices[0], GL_STATIC_DRAW);

	GLuint lego_uvbuffer;
	glGenBuffers(1, &lego_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lego_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, lego_uvs.size() * sizeof(glm::vec2), &lego_uvs[0], GL_STATIC_DRAW);

	GLuint lego_normalbuffer;
	glGenBuffers(1, &lego_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lego_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, lego_normals.size() * sizeof(glm::vec3), &lego_normals[0], GL_STATIC_DRAW);

	GLuint lego_elementbuffer;
	glGenBuffers(1, &lego_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lego_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesLego.size() * sizeof(unsigned short), &indicesLego[0], GL_STATIC_DRAW);

	GLuint lego_colorbuffer;
	glGenBuffers(1, &lego_colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lego_colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, lego_color.size() * sizeof(glm::vec3), &lego_color[0], GL_STATIC_DRAW);
	*/
#pragma endregion
#pragma region cube buffers

	/*GLuint normalTexture = loadBMP_custom("./img/normal.bmp");

	GLuint normalTextureID = glGetUniformLocation(program, "normalTexture");

	GLuint ModelView3x3MatrixID = glGetUniformLocation(program, "MV3x3");

	std::vector<glm::vec3> incube_vertices;
	std::vector<glm::vec2> incube_uvs;
	std::vector<glm::vec3> incube_normals;
	std::vector<glm::vec3> incube_tangents;
	std::vector<glm::vec3> incube_bitangents;

	std::vector<glm::vec3> cube_vertices;
	std::vector<glm::vec2> cube_uvs;
	std::vector<glm::vec3> cube_normals;
	std::vector<glm::vec3> cube_tangents;
	std::vector<glm::vec3> cube_bitangents;

	std::vector<unsigned short> indicesCube;

	if (!loadOBJ("resources/models/cube.obj", incube_vertices, incube_uvs, incube_normals)) {
		return -1;
	}
	// Reset the position
	for (int i = 0; i < incube_vertices.size(); i++) {
		incube_vertices[i] += glm::vec3(0, -2, 0);
	}

	computeTangentBasis(incube_vertices, incube_uvs, incube_normals, incube_tangents, incube_bitangents);

	indexVBO_TBN(incube_vertices, incube_uvs, incube_normals, incube_tangents, incube_bitangents, indicesCube, cube_vertices, cube_uvs, cube_normals, cube_tangents, cube_bitangents);


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

	GLuint cube_elementbuffer;
	glGenBuffers(1, &cube_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCube.size() * sizeof(unsigned short), &indicesCube[0], GL_STATIC_DRAW);

	GLuint cube_tangentbuffer;
	glGenBuffers(1, &cube_tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_tangents.size() * sizeof(glm::vec3), &cube_tangents[0], GL_STATIC_DRAW);

	GLuint cube_bitangentbuffer;
	glGenBuffers(1, &cube_bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_bitangents.size() * sizeof(glm::vec3), &cube_bitangents[0], GL_STATIC_DRAW);
	*/
#pragma endregion

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Enable transparencies
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

#pragma region draw lego

		/*glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, lego_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, lego_normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, lego_colorbuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, lego_vertices.size() * sizeof(glm::vec3));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lego_elementbuffer);


		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indicesLego.size(), GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		*/

		legoMesh.drawMesh();

#pragma endregion

#pragma region cube
		/*
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

		// Bind our normal texture in Texture Unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		// Set our "Normal    TextureSampler" sampler to user Texture Unit 0
		glUniform1i(normalTextureID, 1);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cube_uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, cube_normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, cube_tangentbuffer);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, cube_bitangentbuffer);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indicesCube.size(), GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		*/
#pragma endregion

		computeMatricesFromInputs(window);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat3 ModelView3x3Matrix = glm::mat3(ViewMatrix * ModelMatrix); // Take the upper-left part of ModelViewMatrix
		glm::mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;


		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		//glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
		glUniform3f(LightPositionID, light.position.x, light.position.y, light.position.z);
		glUniform3f(LightColorID, light.color.r, light.color.g, light.color.b);
		glUniform1f(LightIntensityID, light.intensity);

		/*glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);*/

		
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
