// walkmap creation file
#include <walkmap.hpp>
#include <utils.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <ctgmath>

// process a bbox in a walkmap
void processObject(Object* owner, std::vector<BoundingBox*>* bboxes, std::vector<Object*>* scene, std::vector<uint32_t>* sortedByHeight, uint32_t heightIndex, WalkmapSettings& settings){
	// any new boxes created below get pushed to here and then pushed to bboxes at the end (to avoid screwing with the loop)
	std::vector<BoundingBox*> newBboxes;
	
	// loop through each bbox
	for(uint32_t i = 0; i < bboxes->size(); i++){
		// get bbox
		BoundingBox* bbox1 = bboxes->at(i);
		
		// loop through every object ahead of owner
		for(uint32_t j = heightIndex; j < sortedByHeight->size(); j++){
			// get object
			Object* obj2 = scene->at( sortedByHeight->at(j) );
			
			// ignore potential intersection if the distance from the bottom face of obj2 and the top face of obj1 exceeds the player height (then obj2 has no effect on obj1's walkable space)
			if( (obj2->position.y-obj2->scale.y/2.f) - (owner->position.y+owner->scale.y/2.f) >= settings.playerHeight ) continue;
			
			// create bounding box if necessary
			if(obj2->bboxes->size() <= 0){
				*obj2->bboxes = {objToBbox(obj2)};
			}
			
			// objects shouldn't have more than one bbox until they're processed, so we can just assume that the first bbox in the vector is the only one for now
			BoundingBox* bbox2 = obj2->bboxes->at(0);
			
			// check for intersection
			if(!bboxIntersection(bbox1, bbox2)) continue;
			
			// remove bbox1 from boxes (replaced by splitBoxes)
			bboxes->erase(bboxes->begin() + i);
			
			// split bbox
			std::vector<BoundingBox*> splitBoxes;
			splitBbox(&splitBoxes, bbox1, bbox2);
			
			// if the distance from each obj's top faces is <= the step height, mark all split bboxes as adjacent to bbox2 (to allow the player to step up to it)
			if( nearly_less_or_eq(bbox2->position.y - bbox1->position.y, settings.stepHeight) ){
				for(uint32_t k = 0; k < splitBoxes.size(); k++){
					markAdjacent(splitBoxes.at(k), bbox2);
				}
				
				// mark object as reachable (can be reached from this object)
				obj2->reachable = true;
			}
			
			// process new bboxes
			j++; // increment j to ignore the object we just went over
			processObject(owner, &splitBoxes, scene, sortedByHeight, j, settings);
			
			// push new boxes to newBboxes
			newBboxes.insert(newBboxes.end(), splitBoxes.begin(), splitBoxes.end());
			
			// destroy bbox1
			destroyBbox(bbox1);
			
			// we break here because we don't need to check any more objects with bbox1; the bboxes from the split check the rest
			break;
		}
	}
	
	// copy new boxes to bboxes
	bboxes->insert(bboxes->end(), newBboxes.begin(), newBboxes.end());
}

// generate walkmap from vector of objects into a vector of bounding boxes
void generateWalkmap(WalkmapSettings& settings, std::vector<Object*>* objects, std::vector<BoundingBox*>* walkmap){
	// the vectors below store indexes to the objects vector, indicating information about the object at that index
	
	// each object sorted by least to greatest y value
	// starts with index 0
	std::vector<uint32_t> sortedByHeight = {0};
	
	// each index of this vector is the indexes 
	std::vector<std::vector<uint32_t>> collisionGroups(objects->size(), std::vector<uint32_t>());
	
	printf(" - Sorting objects by height...\n");
	
	// sort objects by height (based on the top face's height)
	// binary sort (kinda)
	for(uint32_t i = 1; i < objects->size(); i++){
		uint32_t low = 0;
		uint32_t high = sortedByHeight.size()-1;
		uint32_t size = high-low+1;
		uint32_t index = size / 2;
		
		Object* u = objects->at(i);
		float us = u->position.y + u->scale.y/2.f;
		
		while(size > 2){
			Object* t = objects->at(sortedByHeight[index]);
			float them = t->position.y + t->scale.y/2.f;
			
			// determine direction of index change
			int32_t direction = signum(us - them);
			
			if(direction == -1){
				high = index-1;  
			} else if(direction == 1){
				low = index+1;
			} else {
				// exit early
				size = 0;
				break;
			}
			
			size = high-low+1;
			
			// determine change in index
			uint32_t change = size/2;
			
			index += direction * (1+change);
		}
		
		// sort using the remaining components, should be either 1 or 2
		if(size == 1){
			float them = objects->at(sortedByHeight[index])->position.y + objects->at(sortedByHeight[index])->scale.y/2.f;
			
			index += us > them;
		} else if(size == 2){
			// somewhat of a hack to make math work
			index = high;
			
			float them1 = objects->at(sortedByHeight[low])->position.y + objects->at(sortedByHeight[low])->scale.y/2.f;
			float them2 = objects->at(sortedByHeight[high])->position.y + objects->at(sortedByHeight[high])->scale.y/2.f;

			index += (us > them2) - (us < them1);
		}

		sortedByHeight.insert(sortedByHeight.begin()+index, i);
	}
	
	//printf(" - Sorting objects by intersections...\n");
	printf(" - Calculating walkable space...\n");
	
	// calculate walkable space
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		// get this object
		Object* obj1 = objects->at(sortedByHeight[i]);
		
		// if this isn't element 0 and the object is unreachable, don't bother
		if(!obj1->reachable && i != 0){
			continue;
		}
		
		// create a bounding box for this object if necessary
		if(obj1->bboxes->size() <= 0){
			BoundingBox* bbox1 = objToBbox(obj1);
			*obj1->bboxes = {bbox1};
		}
		
		// recursively parse the object into bboxes
		if(i+1 < sortedByHeight.size()) processObject(obj1, obj1->bboxes, objects, &sortedByHeight, i+1, settings);
		
		// print
		printf("bboxes for object %d (%f, %f, %f):\n", sortedByHeight[i], obj1->position.x, obj1->position.y, obj1->position.z);
		
		for(uint32_t i = 0; i < obj1->bboxes->size(); i++){
			BoundingBox* box = obj1->bboxes->at(i);
			
			printf("%d: pos: %f, %f, %f, size: %f, %f\n", i+1, box->position.x, box->position.y, box->position.z, box->size.x, box->size.y);
		}
		
		printf("\n");
		
		// write all of the boxes to the walkmap
		walkmap->insert(walkmap->end(), obj1->bboxes->begin(), obj1->bboxes->end());
	}
	
	// sort objects by intersections
	/*for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		// create collision group
		std::vector<uint32_t> collisionGroup;
		
		// get this object
		Object& obj1 = objects->at(sortedByHeight[i]);
		
		// loop through every object ahead of this one in height
		for(uint32_t j = i+1; j < sortedByHeight.size(); j++){
			Object& obj2 = objects->at(sortedByHeight[j]);
			
			// ignore potential intersection if the distance from the bottom face of obj2 and the top face of obj1 exceeds the player height (then obj2 has no effect on obj1's walkable space
			if( (obj2.position.y-obj2.scale.y/2.f) - (obj1.position.y+obj1.scale.y/2.f) >= settings.playerHeight ) continue;
			
			glm::vec2 posi = glm::vec2(obj1.position.x, obj1.position.z);
			glm::vec2 sizi = glm::vec2(obj1.scale.x, obj1.scale.z) + glm::vec2(settings.playerRadius); // add player radius to account for objects slightly outside of surface area, but that could still have an effect on walkable space because of the size of the user
			glm::vec2 posj = glm::vec2(obj2.position.x, obj2.position.z);
			glm::vec2 sizj = glm::vec2(obj2.scale.x, obj2.scale.z) + glm::vec2(settings.playerRadius); // ditto
			
			// check for intersection (excluding y axis)
			if(bboxIntersection( posi, sizi, posj, sizj )){
				collisionGroup.push_back(sortedByHeight[j]);
			}
		}
		
		// write collision group to collision groups
		collisionGroups[sortedByHeight[i]] = collisionGroup;
	}*/
	
	// print collision groups
	/*for(uint32_t i = 0; i < collisionGroups.size(); i++){
		printf("group %d: ", i);
		
		for(uint32_t j = 0; j < collisionGroups[i].size(); j++){
			printf("%d, ", collisionGroups[i][j]);
		}
		
		printf("\n");
	}
	
	printf(" - Calculating walkable space...\n");
	
	// starting from the lowest object, determine walkable space
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		// get index of main object
		uint32_t index = sortedByHeight[i];
		
		// get main object
		Object& main = objects->at(index);
		
		// loop through every object intersecting with this one
		std::vector<uint32_t>& intersections = collisionGroups[i];
		
		for(uint32_t j = 0; j < intersections.size(); j++){
			// get intersecting object
			Object& intersect = objects->at(intersections[j]);
			
			// 
		}
	}*/
}

// create bounding box with 2d position
BoundingBox* createBbox(glm::vec2 p, glm::vec2 s){
	return createBbox(glm::vec3(p.x, 0, p.y), s);
}

// create bounding box with 3d position
BoundingBox* createBbox(glm::vec3 p, glm::vec2 s){
	// create 2d corners
	glm::vec2 UL = glm::vec2( (p.x-s.x/2.f), (p.z-s.y/2.f) );
	glm::vec2 UR = glm::vec2( (p.x+s.x/2.f), (p.z-s.y/2.f) );
	glm::vec2 BL = glm::vec2( (p.x-s.x/2.f), (p.z+s.y/2.f) );
	glm::vec2 BR = glm::vec2( (p.x+s.x/2.f), (p.z+s.y/2.f) );
	
	// allocate space for box
	BoundingBox* box = allocateMemoryForType<BoundingBox>();
	box->position = p;
	box->size = s;
	
	box->UL = UL;
	box->UR = UR;
	box->BL = BL;
	box->BR = BR;
	
	box->adjacent = new std::vector<BoundingBox*>();
	
	return box;
}

BoundingBox* objToBbox(Object* obj){
	return createBbox( glm::vec3(obj->position.x, obj->position.y + obj->scale.y/2.f, obj->position.z), glm::vec2(obj->scale.x, obj->scale.z) );
}

void destroyBbox(BoundingBox* b){
	delete b->adjacent;
	
	free(b);
}

// FIXME: doesn't account for rotation
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2){
	return (
		p1.x+s1.x/2.f >= p2.x-s2.x/2.f &&
		p1.y+s1.y/2.f >= p2.y-s2.y/2.f &&
		p2.x+s2.x/2.f >= p1.x-s1.x/2.f &&
		p2.y+s2.y/2.f >= p1.y-s1.y/2.f
	);
}

bool bboxIntersection(BoundingBox* b1, BoundingBox* b2){
	return bboxIntersection( glm::vec2(b1->position.x, b1->position.z) , b1->size, glm::vec2(b2->position.x, b2->position.z), b2->size);
}

void markAdjacent(BoundingBox* b1, BoundingBox* b2){
	if(b1->adjacent != NULL) b1->adjacent->push_back(b2);
	if(b2->adjacent != NULL) b2->adjacent->push_back(b1);
}

// splits an AABB into multiple AABBs around a splitter AABB
// returns 1-4 AABBs
void splitBbox(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter){
	// if boxes are not intersecting, return original
	if(!bboxIntersection(original, splitter)){
		*newBoxes = {original};
		return;
	}
	
	// box1
	glm::vec2 s1 = splitter->UR - original->UL;
	glm::vec2 p1 = original->UL + s1/2.f;
	
	BoundingBox* box1 = createBbox(glm::vec3(p1.x, original->position.y, p1.y), s1);
	
	// box2
	glm::vec2 s2 = glm::vec2(original->UR.x - splitter->BR.x, splitter->BR.y - original->UR.y);
	glm::vec2 p2 = glm::vec2(splitter->UR.x + s2.x/2.f, std::min(original->UR.y + s2.y/2.f, original->position.y));
	
	BoundingBox* box2 = createBbox(glm::vec3(p2.x, original->position.y, p2.y), s2);
	
	// box3
	glm::vec2 s3 = original->BR - splitter->BL;
	glm::vec2 p3 = splitter->BL + s3/2.f;
	
	p3.x = std::max(splitter->BL.x + s3.x/2.f, original->position.x);
	
	BoundingBox* box3 = createBbox(glm::vec3(p3.x, original->position.y, p3.y), s3);
	
	// box4
	glm::vec2 s4 = glm::vec2(splitter->UL.x - original->BL.x, original->BL.y - splitter->UL.y);
	glm::vec2 p4 = glm::vec2(std::min(original->BL.x + s4.x/2.f, original->position.x), std::max(splitter->UL.y + s4.y/2.f, original->position.y));
	
	BoundingBox* box4 = createBbox(glm::vec3(p4.x, original->position.y, p4.y), s4);
	
	// assign boxes
	*newBoxes = {box1, box2, box3, box4};
	
	// some final operation to ensure new boxes are valid (some may have negative scale components or be too large)
	for(uint32_t i = 0; i < newBoxes->size(); i++){
		BoundingBox* box = newBoxes->at(i);
		
		// check for negative w/h
		if(box->size.x <= 0 || box->size.y <= 0){
			//printf("box %d bad: ", i);
			//printf("pos: %f, %f, %f, size: %f, %f\n", box->position.x, box->position.y, box->position.z, box->size.x, box->size.y);
			
			// delete box
			destroyBbox(box);
			
			// remove box (kinda)
			(*newBoxes)[i] = NULL;
			
			// prevent skipping
			//i--;
			continue;
		}
		
		// correct large sizes
		box->size.x = std::min(original->size.x, box->size.x);
		box->size.y = std::min(original->size.y, box->size.y);
	}
	
	// FIXME: find a better way to do adjacency
	
	// adjacency
	for(uint32_t i = 0; i < newBoxes->size(); i++){
		BoundingBox* box = newBoxes->at(i);
		
		if(!box) continue;
		
		BoundingBox* after = newBoxes->at( (i+1) % newBoxes->size() );
		
		if(after) markAdjacent(box, after);
	}
	
	// remove null boxes
	for(uint32_t i = 0; i < newBoxes->size(); i++){
		BoundingBox* box = newBoxes->at(i);
		
		if(!box){
			newBoxes->erase(newBoxes->begin() + i);
			i--;
		}
	}
	
	// done
	return;
}