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
	const static uint32_t numSettings = 5;
	
	float playerHeight;
	float playerRadius;
	float stepHeight;
	float maxPlayerSpeed;
	float heightSpeed; // not really a walkmap setting, but a world setting that makes sense to include here
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
	// none of the above is actually true, this was intended to be used for that purpose but ended up being used to keep track of the index when writing the walkmap to a file
	//int32_t splitIndex;
	
	// is this bbox reachable, used in pushBboxes and deleteUnreachable to determine where the object is reached, and then afterward in walkmapToBuffer as an adjacency index
	int32_t reachable;
	
	//bool reachable;
};

void processObject(Object* owner, std::vector<BoundingBox*>* bboxes, std::vector<Object*>* scene, std::vector<uint32_t>* sortedByHeight, uint32_t heightIndex, WalkmapSettings& settings);
void pushBboxes(BoundingBox* bbox);
void pushBboxesNoRecurse(BoundingBox* bbox);
void deleteUnreachable(std::vector<BoundingBox*>* bboxes);
void generateWalkmap(WalkmapSettings& settings, std::vector<Object*>* objects, std::vector<BoundingBox*>* finalWalkmap);
void walkmapToBuffer(std::string& buffer, std::vector<BoundingBox*>* walkmap, WalkmapSettings& settings);
void walkmapToWorld(std::string& buffer, std::vector<BoundingBox*>* walkmap, WalkmapSettings& settings);

BoundingBox* createBbox(glm::vec2 p, glm::vec2 s);
BoundingBox* createBbox(glm::vec3 p, glm::vec2 s);
BoundingBox* createBbox(BoundingBox* original);
BoundingBox* objToBbox(Object* obj);
void generateBboxCorners(BoundingBox* box);
void destroyBbox(BoundingBox* b);
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2);
bool bboxIntersection(BoundingBox* b1, BoundingBox* b2);
void markAdjacent(BoundingBox* b1, BoundingBox* b2);
void moveBbox(BoundingBox* original, glm::vec3 newPosition);
void resizeBbox(BoundingBox* original, glm::vec2 newSize);
void moveAndResizeBbox(BoundingBox* original, glm::vec3 newPosition, glm::vec2 newSize);
void splitBbox(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter);

#endif