// stripped down world parser, only returns required information to create walkmap (object blocks)

#ifndef WALKMAP_WORLD_H
#define WALKMAP_WORLD_H

// includes //
#include <string>
#include <vector>
#include <map>
#include <utility>

#include <glm/glm.hpp>

// structs //

// declare bounding box here so that we can use pointers to it
struct BoundingBox;

// object
struct Object {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	
	std::vector<BoundingBox*>* bboxes;
	
	bool reachable;
	
	std::vector<std::string>* ids;
	
	int32_t children;
};

// scene
struct Scene {
	std::vector<Object*>* objects;
	
	std::map<std::string, std::pair<glm::vec3, glm::vec3>>* modelSizes;
};

// parsing block
struct Block {
	// vector of ids
	std::vector<std::string>* ids;
	
	// vector for string parameters
	std::vector<std::string>* strings;
	
	// vector for num parameters (always floats)
	std::vector<float>* numbers;
	
	// index of parameter being written to
	uint32_t parameterIndex;
	
	// string buffer for parameter
	std::string* parameterBuffer;
};


// methods //
Object* createEmptyObject();
Object* createObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::vector<std::string>* ids);

Scene* parseWorld(const char* file);
bool parseWorldIntoScene(Scene* scene, const char* file);

#endif