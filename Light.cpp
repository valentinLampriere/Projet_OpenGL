#include "Light.h"


Light::Light(glm::vec3 pos, glm::vec3 c, float intensity) {
	this->position = pos;
	this->color = c;
	this->intensity = intensity;
}