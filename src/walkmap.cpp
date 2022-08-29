// walkmap creation file
#include <walkmap.hpp>
#include <utils.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <ctgmath>

// process an object into bboxes recursively
void processObject(Object* owner, std::vector<BoundingBox*>* bboxes, std::vector<Object*>* scene, std::vector<uint32_t>* sortedByHeight, uint32_t heightIndex, WalkmapSettings& settings){
	// any new boxes created below get pushed to here and then pushed to bboxes at the end (to avoid screwing with the loop)
	std::vector<BoundingBox*> newBboxes;
	
	// loop through each bbox
	for(uint32_t i = 0; i < bboxes->size(); i++){
		// get bbox
		BoundingBox* bbox1 = bboxes->at(i);
		
		// ignore if null
		if(bbox1 == NULL) continue;
		
		//printf("sdp: (p: %f, %f, %f, s: %f, %f)\n", bbox1->position.x, bbox1->position.y, bbox1->position.z, bbox1->size.x, bbox1->size.y);
		
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
			
			// can these bboxes be stepped between?
			bool steppable = nearly_less_or_eq(bbox2->position.y - bbox1->position.y, settings.stepHeight);
			
			// check for intersection
			// when checking for intersection we add the player's radius to size in order to account for objects which may not technically be intersecting but would interfere with walkable space.  then, if there is an intersection, bbox1 scale is returned to normal but bbox2 scale is temporarily kept the same to account for its effect on the walkable space.
			// however, we only do this with impassable objects to prevent the player from seeing through them by being too close.  steppable objects are fine.
			
			//if(!steppable) resizeBbox(bbox2, bbox2->size + settings.playerRadius);
			
			//printf("bbox2: (p: %f, %f, %f, s: %f, %f)\n", bbox2->position.x, bbox2->position.y, bbox2->position.z, bbox2->size.x, bbox2->size.y);
			if(!bboxIntersection(bbox1, bbox2)){ 
				//if(!steppable) resizeBbox(bbox2, bbox2->size - settings.playerRadius);
				continue;
			}
			//printf("got intersect\n");
			
			// remove bbox1 from boxes (replaced by splitBoxes)
			bboxes->erase(bboxes->begin() + i);
			i--;
			
			// split bbox
			std::vector<BoundingBox*> splitBoxes;
			splitBbox(&splitBoxes, bbox1, bbox2);
			
			//printf("checking adjacency\n");
			
			// if the distance from each obj's top faces is <= the step height, mark all split bboxes as adjacent to bbox2 (to allow the player to step up to it)
			//printf("%d: %f, %f, %d\n", sortedByHeight->at(j), bbox2->position.y - bbox1->position.y, settings.stepHeight, steppable);
			if(steppable){
				for(uint32_t k = 0; k < splitBoxes.size(); k++){
					if(splitBoxes.at(k) != NULL) markAdjacent(splitBoxes.at(k), bbox2);
				}
				
				// mark object as reachable (can be reached from this object)
				obj2->reachable = true;
			}
			
			//printf("updating adjacency\n");
			
			// if bbox1 had any adjacent boxes, update those boxes' adjacency
			for(uint32_t k = 0; k < bbox1->adjacent->size(); k++){
				// get adjacent bbox
				BoundingBox* adjacentBbox = bbox1->adjacent->at(k);
				
				// remove bbox1 from adjacentBbox's adjacent vector
				// FIXME: if std::find fails, this will probably seg fault (but std::find shouldn't fail here)
				adjacentBbox->adjacent->erase( std::find(adjacentBbox->adjacent->begin(), adjacentBbox->adjacent->end(), bbox1) );
				
				// add adjacent boxes
				// FIXME: might be able to be made more efficient via splitIndex
				
				for(uint32_t m = 0; m < splitBoxes.size(); m++){
					if(splitBoxes.at(m) == NULL) continue;
					
					if(bboxIntersection(splitBoxes.at(m), adjacentBbox)) markAdjacent(adjacentBbox, splitBoxes.at(m));
				}
				
				/*if(adjacentBbox->splitIndex != -1){
					for(int32_t m = 0; m < 1; m++){
						BoundingBox* other = splitBoxes.at( adjacentBbox->splitIndex+m );
						
						if(other != NULL){ markAdjacent(adjacentBbox, other); }
					}
				}*/
			}
			
			//if(!steppable) resizeBbox(bbox2, bbox2->size - settings.playerRadius);
			
			//printf("processing new bboxes\n");
			
			// destroy bbox1
			destroyBbox(bbox1);
			
			// process new bboxes
			j++; // increment j to ignore the object we just went over
			processObject(owner, &splitBoxes, scene, sortedByHeight, j, settings);
			
			// push new boxes to newBboxes
			newBboxes.insert(newBboxes.end(), splitBoxes.begin(), splitBoxes.end());
			
			// we break here because we don't need to check any more objects with bbox1; the bboxes from the split check the rest
			break;
		}
	}
	
	// copy new boxes to bboxes
	bboxes->insert(bboxes->end(), newBboxes.begin(), newBboxes.end());
}

// push bboxes to a walkmap
void pushBboxes(BoundingBox* bbox){
	// ignore null
	if(bbox == NULL) return;
	
	// if this bbox has already been reached, ignore
	if( bbox->reachable ) return;
	
	// mark for non-deletion
	bbox->reachable = 1;
	
	// check for null adjacent
	if(bbox->adjacent == NULL) return;
	
	// iterate through adjacent boxes and push them to walkmap
	for(uint32_t i = 0; i < bbox->adjacent->size(); i++){
		pushBboxes(bbox->adjacent->at(i));
	}
	
	// done
}

// non-recursive version of pushBboxes (this avoids stack overflow with too many bboxes)
// this isn't all that bad efficiency wise compared to pushBboxes, so I'll probably stick to this.  the worst part is just the vector reallocation, but the lack of stack overflow makes up for it
void pushBboxesNoRecurse(BoundingBox* bbox){
	std::vector<BoundingBox*> parents = {bbox};
	
	while(parents.size() > 0){
		BoundingBox* active = parents.back();
		uint32_t index = parents.size()-1;
		
		active->reachable = true;
		
		if(active->adjacent){
			for(uint32_t i = 0; i < active->adjacent->size(); i++){
				BoundingBox* adjacent = active->adjacent->at(i);
				if(!adjacent->reachable) parents.push_back(adjacent);
			}
		}
		
		parents.erase(parents.begin() + index);
	}
}

void deleteUnreachable(std::vector<BoundingBox*>* bboxes){
	for(uint32_t i = 0; i < bboxes->size(); i++){
		if(!bboxes->at(i)->reachable){
			bboxes->erase( bboxes->begin() + i );
			i--;
		}
	}
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
	
	// print sorted
	
	/*
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		printf("%d, ", sortedByHeight[i]);
	}
	
	printf("\n");
	*/
	
	// calculate walkable space
	printf(" - Calculating walkable space...\n");
	
	// loop through each object and call process
	for(uint32_t i = 0; i < sortedByHeight.size(); i++){
		// get this object
		Object* obj1 = objects->at(sortedByHeight[i]);
		
		if(!obj1 || !obj1->bboxes) continue;
		
		// create a bounding box for this object if necessary
		if(obj1->bboxes->size() <= 0){
			BoundingBox* bbox1 = objToBbox(obj1);
			*obj1->bboxes = {bbox1};
		}
		
		// recursively parse the object into bboxes
		if(i+1 < sortedByHeight.size()) processObject(obj1, obj1->bboxes, objects, &sortedByHeight, i+1, settings);
		
		/*
		// print
		printf("bboxes for object %d (%f, %f, %f):\n", sortedByHeight[i], obj1->position.x, obj1->position.y, obj1->position.z);
		
		
		for(uint32_t i = 0; i < obj1->bboxes->size(); i++){
			BoundingBox* box = obj1->bboxes->at(i);
			
			//if(box == NULL) continue;
			if(box == NULL){ printf("NULL box\n"); continue; }
			
			printf("%d: pos: %f, %f, %f, size: %f, %f, numAdjacent: %d\n", i+1, box->position.x, box->position.y, box->position.z, box->size.x, box->size.y, box->adjacent->size());
		}
		
		printf("\n");
		*/
		
		// write all of the boxes to the walkmap
		walkmap->insert(walkmap->end(), obj1->bboxes->begin(), obj1->bboxes->end());
	}
	
	// strip null boxes
	for(uint32_t i = 0; i < walkmap->size(); i++){
		if(walkmap->at(i) == NULL){
			walkmap->erase( walkmap->begin()+i );
			i--;
		}
	}
	
	if(walkmap->size() < 1){
		printf("No boxes were generated.\n");
		return;
	}
	
	// eliminate unreachable boxes
	//printf(" - Eliminating unreachable boxes...\n");
	printf(" - Determining reachable boxes...\n");
	
	// recurse through all bboxes indirectly adjacent to the first one
	//pushBboxes(walkmap->at(0));
	pushBboxesNoRecurse(walkmap->at(0));
	
	// remove unreachable
	printf(" - Removing unreachable boxes...\n");
	
	deleteUnreachable(walkmap);
}

// convert walkmap (vector of BoundingBox*s) to a string to be written to a file
void walkmapToBuffer(std::string& buffer, std::vector<BoundingBox*>* walkmap, WalkmapSettings& settings){
	// .walkmap files are just .world files without anything extra
	// the block syntax for walkmap boxes is as follows:
	/*
		~[x,y,z, w,d]
	*/
	
	const char delimiter = '~';
	const char settingsDelimiter = '@';
	const char parameterDelimiter = ',';
	const char blockOpen = '[';
	const char blockClose = ']';

	// write settings to buffer
	uint32_t numSettings = WalkmapSettings::numSettings;
	
	buffer += settingsDelimiter;
	buffer += blockOpen;
	
	for(uint32_t i = 0; i < numSettings; i++){
		buffer += std::to_string( (&settings.playerHeight)[i] ) + ",";
	}
	
	buffer += blockClose;
	buffer += '\n';
	
	// next iterate through walkmap and assign each box an index (overriding the splitIndex value because I'm lazy)
	// the walkmap uses indexes for adjacency information
	for(uint32_t i = 0; i < walkmap->size(); i++){
		walkmap->at(i)->reachable = i;
	}
	
	for(uint32_t i = 0; i < walkmap->size(); i++){
		// temp buffer for block
		std::string blockBuffer;
		
		// add delimiter
		blockBuffer += delimiter;
		
		// add block open
		blockBuffer += blockOpen;
		
		// get box
		BoundingBox* box = walkmap->at(i);
		
		// add floats to block buffer
		for(uint32_t j = 0; j < 5; j++){
			float f = (&box->position[0])[j];
			
			blockBuffer += std::to_string(f) + parameterDelimiter;
		}
		
		// add adjacent boxes
		for(uint32_t j = 0; j < box->adjacent->size(); j++){
			blockBuffer += std::to_string(box->adjacent->at(j)->reachable) + parameterDelimiter;
		}
		
		// add block close
		blockBuffer += blockClose;
		
		// write to buffer
		buffer += blockBuffer + '\n';
	}
}

void walkmapToWorld(std::string& buffer, std::vector<BoundingBox*>* walkmap, WalkmapSettings& settings){
	const char delimiter = '$';
	const char settingsDelimiter = '@';
	const char parameterDelimiter = ',';
	const char blockOpen = '[';
	const char blockClose = ']';

	buffer = "# texture initialization blocks\n\n%[./res/textures/grid.png, default]\n\n# vertex data initialization blocks\n\n*[cube, cube]\n\n# light blocks\n\n&[0, 0, 0,     1, 1, 1,    1, 0, 0,     0.8, 0]\n\n# bbox object blocks\n\n";

	for(uint32_t i = 0; i < walkmap->size(); i++){
		// temp buffer for block
		std::string blockBuffer;
		
		// add delimiter
		blockBuffer += delimiter;
		
		// add block open
		blockBuffer += blockOpen;
		
		// get box
		BoundingBox* box = walkmap->at(i);
		
		// add floats to block buffer
		for(uint32_t j = 0; j < 3; j++){
			float f = (&box->position[0])[j];
			
			blockBuffer += std::to_string(f) + parameterDelimiter;
		}
		
		// add zero for rotation
		blockBuffer += "0,0,0,";
		
		blockBuffer += std::to_string(box->size.x) + ",0," + std::to_string(box->size.y) + ",default,cube";
		
		// add block close
		blockBuffer += blockClose;
		
		// write to buffer
		buffer += blockBuffer + '\n';
	}
}

// create bounding box with 2d position
BoundingBox* createBbox(glm::vec2 p, glm::vec2 s){
	return createBbox(glm::vec3(p.x, 0, p.y), s);
}

// create bounding box with 3d position
BoundingBox* createBbox(glm::vec3 p, glm::vec2 s){
	// allocate space for box
	BoundingBox* box = allocateMemoryForType<BoundingBox>();
	box->position = p;
	box->size = s;
	
	generateBboxCorners(box);
	
	box->adjacent = new std::vector<BoundingBox*>();
	
	//box->splitIndex = -1;
	box->reachable = 0;
	
	return box;
}

BoundingBox* createBbox(BoundingBox* original){
	return createBbox(original->position, original->size);
}

BoundingBox* objToBbox(Object* obj){
	return createBbox( glm::vec3(obj->position.x, obj->position.y + obj->scale.y/2.f, obj->position.z), glm::vec2(obj->scale.x, obj->scale.z) );
}

void generateBboxCorners(BoundingBox* box){
	// create 2d corners
	glm::vec2 UL = glm::vec2( (box->position.x-box->size.x/2.f), (box->position.z-box->size.y/2.f) );
	glm::vec2 UR = glm::vec2( (box->position.x+box->size.x/2.f), (box->position.z-box->size.y/2.f) );
	glm::vec2 BL = glm::vec2( (box->position.x-box->size.x/2.f), (box->position.z+box->size.y/2.f) );
	glm::vec2 BR = glm::vec2( (box->position.x+box->size.x/2.f), (box->position.z+box->size.y/2.f) );
	
	box->UL = UL;
	box->UR = UR;
	box->BL = BL;
	box->BR = BR;
}

void destroyBbox(BoundingBox* b){
	delete b->adjacent;
	
	free(b);
}

// FIXME: doesn't account for rotation
bool bboxIntersection(glm::vec2 p1, glm::vec2 s1, glm::vec2 p2, glm::vec2 s2){
	return (
		/*p1.x+s1.x/2.f >= p2.x-s2.x/2.f &&
		p1.y+s1.y/2.f >= p2.y-s2.y/2.f &&
		p2.x+s2.x/2.f >= p1.x-s1.x/2.f &&
		p2.y+s2.y/2.f >= p1.y-s1.y/2.f*/
		nearly_greater_or_eq(p1.x+s1.x/2.f, p2.x-s2.x/2.f) &&
		nearly_greater_or_eq(p1.y+s1.y/2.f, p2.y-s2.y/2.f) &&
		nearly_greater_or_eq(p2.x+s2.x/2.f, p1.x-s1.x/2.f) &&
		nearly_greater_or_eq(p2.y+s2.y/2.f, p1.y-s1.y/2.f)
	);
}

bool bboxIntersection(BoundingBox* b1, BoundingBox* b2){
	return bboxIntersection( glm::vec2(b1->position.x, b1->position.z) , b1->size, glm::vec2(b2->position.x, b2->position.z), b2->size);
}

void markAdjacent(BoundingBox* b1, BoundingBox* b2){
	if(b1->adjacent != NULL) b1->adjacent->push_back(b2);
	if(b2->adjacent != NULL) b2->adjacent->push_back(b1);
}

void moveBbox(BoundingBox* original, glm::vec3 newPosition){
	moveAndResizeBbox(original, newPosition, original->size);
}

void resizeBbox(BoundingBox* original, glm::vec2 newSize){
	moveAndResizeBbox(original, original->position, newSize);
}

void moveAndResizeBbox(BoundingBox* original, glm::vec3 newPosition, glm::vec2 newSize){
	original->position = newPosition;
	original->size = newSize;
	
	generateBboxCorners(original);
}

// splits an AABB into multiple AABBs around a splitter AABB
// newBoxes will always have either 1 value (if the original is returned) or 4 values, some of which may be NULL
void splitBbox(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter){
	// if boxes are not intersecting, return original
	if(!bboxIntersection(original, splitter)){
		*newBoxes = {createBbox(original)};
		return;
	}
	
	//printf("original: UL: %f, %f, UR: %f, %f, BL: %f, %f, BR: %f, %f\n", original->UL.x, original->UL.y, original->UR.x, original->UR.y, original->BL.x, original->BL.y, original->BR.x, original->BR.y);
	//printf("splitter: UL: %f, %f, UR: %f, %f, BL: %f, %f, BR: %f, %f\n", splitter->UL.x, splitter->UL.y, splitter->UR.x, splitter->UR.y, splitter->BL.x, splitter->BL.y, splitter->BR.x, splitter->BR.y);
	
	/*
	var w1 = n_UR.x - og_UL.x;
	var h1 = n_UR.y - og_UL.y;
	
	var box1 = createBox(Math.min(og_UL.x + w1/2, og.x), og_UL.y + h1/2, w1, h1);
	*/
	
	// box1
	glm::vec2 s1 = splitter->UR - original->UL;
	glm::vec2 p1 = original->UL + s1/2.f;
	
	p1.x = std::min(p1.x, original->position.x);
	
	BoundingBox* box1 = createBbox(glm::vec3( p1.x, original->position.y, p1.y), s1);
	
	/*
	var w2 = og_UR.x - n_BR.x;
	var h2 = n_BR.y - og_UR.y;
	
	var box2 = createBox(n_UR.x + w2/2, Math.min(og_UR.y + h2/2, og.y), w2, h2);
	*/
	
	// box2
	glm::vec2 s2 = glm::vec2(original->UR.x - splitter->BR.x, splitter->BR.y - original->UR.y);
	glm::vec2 p2 = glm::vec2(splitter->UR.x + s2.x/2.f, original->UR.y + s2.y/2.f);
	
	p2.y = std::min(p2.y, original->position.z);
	
	BoundingBox* box2 = createBbox(glm::vec3(p2.x, original->position.y, p2.y), s2);
	
	/*
	var w3 = og_BR.x - n_BL.x;
	var h3 = og_BR.y - n_BL.y;
	
	var box3 = createBox(Math.max(n_BL.x + w3/2, og.x), n_BL.y + h3/2, w3, h3);
	*/
	
	// box3
	glm::vec2 s3 = original->BR - splitter->BL;
	glm::vec2 p3 = splitter->BL + s3/2.f;
	
	p3.x = std::max(splitter->BL.x + s3.x/2.f, original->position.x);
	
	BoundingBox* box3 = createBbox(glm::vec3(p3.x, original->position.y, p3.y), s3);
	
	/*
	var w4 = n_UL.x - og_BL.x;
	var h4 = og_BL.y - n_UL.y;
	
	var box4 = createBox(Math.min(og_BL.x + w4/2, og.x), Math.max(n_UL.y + h4/2, og.y), w4, h4);
	*/
	
	// box4
	glm::vec2 s4 = glm::vec2(splitter->UL.x - original->BL.x, original->BL.y - splitter->UL.y);
	glm::vec2 p4 = glm::vec2(original->BL.x + s4.x/2.f, splitter->UL.y + s4.y/2.f);
	
	p4.x = std::min(p4.x, original->position.x);
	p4.y = std::max(p4.y, original->position.z);
	
	BoundingBox* box4 = createBbox(glm::vec3(p4.x, original->position.y, p4.y), s4);
	
	// assign boxes
	*newBoxes = {box1, box2, box3, box4};
	
	// some final operation to ensure new boxes are valid (some may have negative scale components or be too large)
	for(uint32_t i = 0; i < newBoxes->size(); i++){
		BoundingBox* box = newBoxes->at(i);
		
		// check for negative w/h
		if(box->size.x <= 0 || box->size.y <= 0){
			// delete box
			destroyBbox(box);
			
			// remove box (kinda)
			newBoxes->at(i) = NULL;
			
			// prevent skipping
			//i--;
			continue;
		}
		
		//box->splitIndex = i;
		
		// correct large sizes
		box->size.x = std::min(original->size.x, box->size.x);
		box->size.y = std::min(original->size.y, box->size.y);
		
		generateBboxCorners(box);
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
	/*for(uint32_t i = 0; i < newBoxes->size(); i++){
		BoundingBox* box = newBoxes->at(i);
		
		if(!box){
			newBoxes->erase(newBoxes->begin() + i);
			i--;
		}
	}*/
	
	// done
	return;
}

// splits bboxes into an anvil pattern (long on top and bottom, short on sides)
void splitBboxAnvil(std::vector<BoundingBox*>* newBoxes, BoundingBox* original, BoundingBox* splitter){
}