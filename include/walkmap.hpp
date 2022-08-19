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
	
	// if the bounding box is a result of a call to splitBbox, this int contains the position this bbox held in the newBoxes vector before being checked for adjacency (represents if it was box1, box2, etc.) (otherwise is -1)
	// this is used to make adjacency recalculation as a result of a split faster, because bounding boxes at a certain position in a split are always adjacent to certain bounding boxes in the splitter, should the splitter be split (sorry for the tongue twister)
	int32_t splitIndex;
};

void processObject(Object* owner, std::vector<BoundingBox*>* bboxes, std::vector<Object*>* scene, std::vector<uint32_t>* sortedByHeight, uint32_t heightIndex, WalkmapSettings& settings);
void pushBboxes(BoundingBox* bbox, std::vector<BoundingBox*>* walkmap);
void generateWalkmap(WalkmapSettings& settings, std::vector<Object*>* objects, std::vector<BoundingBox*>* finalWalkmap);
void walkmapToBuffer(std::string& buffer, std::vector<BoundingBox*>* walkmap);

BoundingBox* createBbox(glm::vec2 p, glm::vec2 s);
BoundingBox* createBbox(glm::vec3 p, glm::vec2 s);
BoundingBox* createBbox(BoundingBox* original);
BoundingBox* objToBbox(Object* obj);
void destroyBbox(BoundingBox* b);
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2);
bool bboxIntersection(BoundingBox* b1, BoundingBox* b2);
void markAdjacent(BoundingBox* b1, BoundingBox* b2);
void splitBbox(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter);

#endif