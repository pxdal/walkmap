// stripped down world parser, only returns required information to create walkmap (object blocks)

#ifndef WALKMAP_WORLD_H
#define WALKMAP_WORLD_H

// includes //
#include <string>
#include <vector>

#include <glm/glm.hpp>

// structs //

// object
struct Object {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

// world blocks (hold information about blocks)

// object block
struct ObjectBlock {
	const static uint32_t numFloats = 9;
	const static uint32_t numStrings = 2;
	
	// vector of float parameters
	std::vector<float>* floats;
	
	// vector of string parameters
	std::vector<std::string>* strings;
	
	// index of parameter being written to
	uint32_t parameterIndex;
	
	// string buffer for parameter
	std::string* parameterBuffer;
};


// methods //
void parseWorld(const char* file, std::vector<Object>* objects);

#endif