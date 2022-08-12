// walkmap creation file

#ifndef WALKMAP_WALKMAP_H
#define WALKMAP_WALKMAP_H

// includes //
#include <world.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>

// settings struct
struct WalkmapSettings {
	float playerHeight;
	float playerRadius;
	float stepHeight;
};

// 2d bounding box struct
struct BoundingBox {
	// position and size
	glm::vec3 position;
	glm::vec2 size;
};

bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2);
void generateWalkmap(std::vector<Object>* objects, std::vector<BoundingBox>* walkmap);

#endif