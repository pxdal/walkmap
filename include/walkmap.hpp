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
	
	// corners
	glm::vec2 UL, UR, BL, BR;
	
	// adjacent bounding boxes
	std::vector<BoundingBox*>* adjacent;
};

void processObject(Object* owner, std::vector<BoundingBox*>* bboxes, std::vector<Object*>* scene, std::vector<uint32_t>* sortedByHeight, uint32_t heightIndex, WalkmapSettings& settings);
void generateWalkmap(WalkmapSettings& settings, std::vector<Object*>* objects, std::vector<BoundingBox*>* finalWalkmap);

BoundingBox* createBbox(glm::vec2 p, glm::vec2 s);
BoundingBox* createBbox(glm::vec3 p, glm::vec2 s);
BoundingBox* objToBbox(Object* obj);
void destroyBbox(BoundingBox* b);
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2);
bool bboxIntersection(BoundingBox* b1, BoundingBox* b2);
void markAdjacent(BoundingBox* b1, BoundingBox* b2);
void splitBbox(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter);

#endif