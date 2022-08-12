// walkmap creation file
#include <walkmap.hpp>
#include <utils.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <ctgmath>

// generate walkmap from vector of objects into a vector of bounding boxes
void generateWalkmap(std::vector<Object>* objects, std::vector<BoundingBox>* walkmap){
	// the vectors below store indexes to the objects vector, indicating information about the object at that index
	
	// each object sorted by least to greatest y value
	// starts with index 0
	std::vector<uint32_t> sortedByHeight = {0};
	
	// each index of this vector is the indexes 
	std::vector<std::vector<uint32_t>> collisionGroups(objects->size(), std::vector<uint32_t>());
	
	// sort objects by height
	// binary sort (kinda)
	for(uint32_t i = 1; i < objects->size(); i++){
		uint32_t low = 0;
		uint32_t high = sortedByHeight.size()-1;
		uint32_t size = high-low+1;
		uint32_t index = size / 2;
		
		float us = objects->at(i).position.y;
		
		while(size > 2){
			float them = objects->at(sortedByHeight[index]).position.y;
			
			// determine direction of index change
			int32_t direction = signum(us - them);
			
			if(direction == -1){
				high = index-1;  
			} else {
				low = index+1;
			}

			size = high-low+1;
			
			// determine change in index
			uint32_t change = size/2;

			index += direction * (1+change);
		}
		
		// sort using the remaining components, should be either 1 or 2
		if(size == 1){
			index += us > objects->at(sortedByHeight[index]).position.y;
		} else if(size == 2){
			// somewhat of a hack to make math work
			index = high;
			
			float them1 = objects->at(sortedByHeight[low]).position.y;
			float them2 = objects->at(sortedByHeight[high]).position.y;

			index += (us > them2) - (us < them1);
		}

		sortedByHeight.insert(sortedByHeight.begin()+index, i);
	}
	
	/*
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		uint32_t index = sortedByHeight[i];
		
		printf("%d (obj at %f, %f, %f)\n", index, objects->at(index).position.x, objects->at(index).position.y, objects->at(index).position.z);
		//printf("%d ", index);
	}
	
	printf("\n");*/
	
	// sort objects by intersections
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		// create collision group
		std::vector<uint32_t> collisionGroup;
		
		// get this object
		Object obj1 = objects->at(sortedByHeight[i]);
		
		// loop through every object ahead of this one in height
		for(uint32_t j = i+1; j < sortedByHeight.size(); j++){
			Object obj2 = objects->at(sortedByHeight[j]);
			
			// not same height though
			if(obj1.position.y == obj2.position.y) continue;
			
			glm::vec2 posi = glm::vec2(obj1.position.x, obj1.position.z);
			glm::vec2 sizi = glm::vec2(obj1.scale.x, obj1.scale.z);
			glm::vec2 posj = glm::vec2(obj2.position.x, obj2.position.z);
			glm::vec2 sizj = glm::vec2(obj2.scale.x, obj2.scale.z);
			
			// check for intersection (excluding y axis)
			if(bboxIntersection( posi, sizi, posj, sizj )){
				collisionGroup.push_back(sortedByHeight[j]);
			}
		}
		
		// write collision group to collision groups
		collisionGroups[sortedByHeight[i]] = collisionGroup;
	}
	
	for(uint32_t i = 0; i < collisionGroups.size(); i++){
		printf("\ngroup %d (obj at %f, %f, %f): ", i, objects->at(i).position.x, objects->at(i).position.y, objects->at(i).position.z);
		
		for(uint32_t j = 0; j < collisionGroups[i].size(); j++){
			printf("%d, ", collisionGroups[i][j]);
		}
		
		printf("\n");
	}
}

// FIXME: doesn't account for rotation
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2){
	return (
		p1.x+s1.x/2 >= p2.x-s2.x/2 &&
		p1.y+s1.y/2 >= p2.y-s2.y/2 &&
		p2.x+s2.x/2 >= p1.x-s1.x/2 &&
		p2.y+s2.y/2 >= p1.y-s1.y/2
	);
}