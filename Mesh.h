#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include "vbo_indexer.h"
#include "globals.h"

class Mesh {
private:
	std::vector<glm::vec3> in_vertices;
	std::vector<glm::vec2> in_uvs;
	std::vector<glm::vec3> in_normals;
	std::vector<glm::vec3> in_tangents;
	std::vector<glm::vec3> in_bitangents;
	std::vector<unsigned short> indices;

	bool hasColor;
	bool hasReindex;

	bool loadOBJ(char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals);
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::vec3> color;

	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint elementbuffer;
	GLuint colorbuffer;
	GLuint tangentbuffer;
	GLuint bitangentbuffer;

	bool loadMesh(char* path, Mesh* loadedMesh, bool reindexVertices = true);
	void setColor(glm::vec3 color);
	void drawMesh();
};