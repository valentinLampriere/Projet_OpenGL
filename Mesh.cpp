#include "Mesh.h"

bool Mesh::loadOBJ(char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals) {
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

bool Mesh::loadMesh(char* path, Mesh* loadedMesh, bool reindexVertices) {
	if (!loadOBJ("resources/models/lego2.obj", this->in_vertices, this->in_uvs, this->in_normals)) {
		return false;
	}

	if (reindexVertices) {
		indexVBO(this->in_vertices, this->in_uvs, this->in_normals, this->indices, this->vertices, this->uvs, this->normals);
	} else {
		this->vertices = this->in_vertices;
		this->uvs = this->in_uvs;
		this->normals = this->in_normals;
	}

	glGenBuffers(1, &this->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), &this->vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &this->uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, this->uvs.size() * sizeof(glm::vec2), &this->uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &this->normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3), &this->normals[0], GL_STATIC_DRAW);
	
	if (reindexVertices) {
		glGenBuffers(1, &this->elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned short), &this->indices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &this->tangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->tangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, this->tangents.size() * sizeof(glm::vec3), &this->tangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &this->bitangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->bitangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, this->bitangents.size() * sizeof(glm::vec3), &this->bitangents[0], GL_STATIC_DRAW);
	}
}

void Mesh::setColor(glm::vec3 color) {
	for (int i = 0; i < this->vertices.size(); i++) {
		this->color.push_back(color);
	}

	glGenBuffers(1, &this->colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, this->color.size() * sizeof(glm::vec3), &this->color[0], GL_STATIC_DRAW);

	this->hasColor = true;
}

void Mesh::drawMesh() {
	glEnableVertexAttribArray(VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
	glVertexAttribPointer(VERTEX, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(UV);
	glBindBuffer(GL_ARRAY_BUFFER, this->uvbuffer);
	glVertexAttribPointer(UV, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, this->normalbuffer);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (hasColor) {
		glEnableVertexAttribArray(COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, this->colorbuffer);
		glVertexAttribPointer(COLOR, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	glEnableVertexAttribArray(TANGENT);
	glBindBuffer(GL_ARRAY_BUFFER, this->tangentbuffer);
	glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(BITANGENT);
	glBindBuffer(GL_ARRAY_BUFFER, this->bitangentbuffer);
	glVertexAttribPointer(BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// Draw the triangle !
	//glDrawArrays(GL_TRIANGLES, 0, lego_vertices.size() * sizeof(glm::vec3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);


	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT, (void*)0);

	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}