// world parser
#include <world.hpp>
#include <utils.hpp>

#include <cctype>
#include <ctgmath>

// object
Object* createEmptyObject(){
	Object* obj = allocateMemoryForType<Object>();
	
	obj->position = glm::vec3(0);
	obj->rotation = glm::vec3(0);
	obj->scale = glm::vec3(0);
	
	obj->bboxes = new std::vector<BoundingBox*>();
	obj->reachable = false;
	
	return obj;
}

Object* createObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale){
	Object* obj = createEmptyObject();
	
	obj->position = position;
	obj->rotation = rotation;
	obj->scale = scale;
	
	return obj;
}

// world

// parser settings
const char commentDelimiter = '#';
const char parameterDelimiter = ',';
const char blockOpen = '[';
const char blockClose = ']';
const char objectBlockDelimiter = '$';
const char vertexDataBlockDelimiter = '*';
const char modelBlockDelimiter = '+';

// create a block
Block* createBlock(){
	Block* block = allocateMemoryForType<Block>();
	
	// construct members
	block->strings = new std::vector<std::string>();
	block->numbers = new std::vector<float>();
	block->parameterBuffer = new std::string();
	
	block->parameterIndex = 0;
	
	return block;
}

// empties out buffers and vectors, but keeps memory allocated
void emptyBlock(Block* block){
	// erase parameter buffer
	block->parameterBuffer->clear();
	
	// empty vectors
	block->strings->clear();
	block->numbers->clear();
	
	// reset parameter index
	block->parameterIndex = 0;
}

// completely deletes occupied memory
void destroyBlock(Block* block){
	// destruct members
	delete block->strings;
	delete block->numbers;
	delete block->parameterBuffer;
	
	free(block);
}

// block char parser
bool blockCharParser(Block* block, char byte){
	// ensure that block exists, create it if it doesn't
	if(block == NULL){
		// weird problem, give up
		return false;
	}
	
	// add byte to parameter buffer if it's not equal to the parameter delimiter or the close block
	if(byte != parameterDelimiter && byte != blockClose){
		block->parameterBuffer->push_back(byte);
	} else {
		// flush parameter buffer
		if(block->parameterBuffer->length() > 0 && isStringNumber( *block->parameterBuffer )){
			block->numbers->push_back( std::stof(*block->parameterBuffer) );
		} else {
			block->strings->push_back( *block->parameterBuffer );
		}
		
		// reset parameterBuffer
		block->parameterBuffer->clear();
		
		// if parameterIndex is at the maximum value, return true (done)
		if(byte == blockClose){
			// block is done parsing
			return true;
		}
		
		// otherwise, increment parameterIndex
		block->parameterIndex++;
	}
	
	return false;
}

void objectBlockToScene(Block* block, Scene* scene){
	// validate float values
	uint32_t numNums = 9; // I like this variable name
	if(block->numbers->size() != numNums){
		printf("Invalid amount of parameters in an object block (%d numbers present, exactly %d required)\n", block->numbers->size(), numNums);
		return;
	}
	
	// check for keywords
	uint32_t stringParams = block->strings->size();
	
	if(stringParams > 1){
		// loop breaks when there are no more keyword
		bool keywordsLeft = true;
		
		while(keywordsLeft){
			std::string keyword = block->strings->at(stringParams-1);
			
			if(keyword == "nowalk"){
				// ignore this object completely
				// nothing allocated yet so no extra cleanup needed
				return;
			} else if(keyword == "invisible"){
				// invisible is only for the engine's world parser, so just get rid of the keyword
				stringParams--;
			} else {
				keywordsLeft = false;
			}
		}
	}
	
	// create new object
	Object* object = createEmptyObject();
	
	// load some float values
	for(uint32_t i = 0; i < block->numbers->size(); i++){
		(&object->position.x)[i] = block->numbers->at(i);
	}
	
	// load model size + offset and apply corrections
	std::string model = block->strings->at(stringParams-1);
	
	glm::vec3 size = object->scale;
	glm::vec3 modelOffset = scene->modelSizes->at(model).first; // FIXME: catch potential exception
	glm::vec3 modelBounding = scene->modelSizes->at(model).second;
	
	object->scale = size * modelBounding;
	
	// snap rotation to nearest axis and then account for rotation in scale
	for(uint32_t i = 0; i < 3; i++){
		float rot = object->rotation[i];
		
		// snap to closest axis
		rot = fmod(rot, 180.f);
		rot /= 90.f;
		rot = round(rot);
		rot = abs(rot);
		
		// adjust scale
		if(rot == 1){
			// swap scale axes
			uint32_t i1 = ((i-1)+3)%3;
			uint32_t i2 = (i+1)%3;
			
			float s1 = object->scale[i1];
			float s2 = object->scale[i2];
			
			object->scale[i1] = s2;
			object->scale[i2] = s1;
			
			// also swap offset axes
			s1 = modelOffset[i1];
			s2 = modelOffset[i2];
			
			modelOffset[i1] = s2;
			modelOffset[i2] = s1;
		}
	}
	
	object->position += size * modelOffset; // apply scaling to offset as well
	
	//printf("obj->position: %f, %f, %f\nobj->size: %f, %f, %f\n\n", object->position.x, object->position.y, object->position.z, object->scale.x, object->scale.y, object->scale.z);
	
	// push to objects
	scene->objects->push_back(object);
}

void vertexDataBlockToScene(Block* block, Scene* scene){
	// load values
	std::string vertexDataName = block->strings->at(1);
	
	glm::vec3 boundingOffset = glm::vec3(0);
	
	for(uint32_t i = 0; i < 3; i++){
		float v = i < block->numbers->size() ? block->numbers->at(i) : 0.0f;
		
		// I'd like to use simple bracket notation here (boundingSize[i] = v) but glm fails if I do(?)
		// I'm guessing that bracket notation is not meant for assignment, but I don't feel like digging through its documentation to find out
		(&boundingOffset.x)[i] = v;
	}
	
	glm::vec3 boundingSize = glm::vec3(0);
	
	for(uint32_t i = 3; i < 6; i++){
		float v = i < block->numbers->size() ? block->numbers->at(i) : 0.0f;
		
		(&boundingSize.x)[i-3] = v;
	}
	
	(*scene->modelSizes)[vertexDataName] = std::make_pair(boundingOffset, boundingSize);
}

Scene* createScene(){
	Scene* scene = allocateMemoryForType<Scene>();
	
	scene->objects = new std::vector<Object*>();
	scene->modelSizes = new std::map<std::string, std::pair<glm::vec3, glm::vec3>>();
	
	return scene;
}

// parse world (create new scene)
Scene* parseWorld(const char* file){
	Scene* scene = createScene();
	
	parseWorldIntoScene(scene, file);
	
	return scene;
}

// parse world into an existing scene object
void parseWorldIntoScene(Scene* scene, const char* file){
	// file buffer
	char* fileBuffer = read_entire_file(file);
	
	if(fileBuffer == NULL){
		printf("Invalid path for world %s\n", file);
		return;
	}
	
	// settings
	char blockDelimiters[] = {objectBlockDelimiter, vertexDataBlockDelimiter, modelBlockDelimiter};
	
	// control states
	bool parsingBlock = false;
	bool ignoringUntilNextLine = false;
	int32_t blockParsing = -1;
	Block* blockBuffer = createBlock();
	
	// pointers to char parsers
	// first param = pointer to block (void* here to be valid for all pointers)
	// second param = char to parse
	// returns false if the block hasn't finished parsing and true if it has
	void (*blockParsers[3])(Block*,Scene*) {objectBlockToScene, vertexDataBlockToScene, vertexDataBlockToScene};
	
	// loop through each byte
	char byte;
	char lastByte;
	uint32_t byteIndex = 0;
	
	do {
		// load byte
		byte = fileBuffer[byteIndex];
		if(byteIndex > 0) lastByte = fileBuffer[byteIndex-1];
		
		uint32_t index = byteIndex;
		byteIndex++;
		
		// if currently in a comment, check for newline
		if(ignoringUntilNextLine){
			// check for newline
			if(byte == '\n'){
				ignoringUntilNextLine = false;
			}
			
			continue;
		}
		
		// check for whitespace (ignored)
		if(isspace(byte)) continue;
		
		// check for hashtag (comment, ignores)
		if( (lastByte == '\n' || index == 0) && byte == commentDelimiter){
			ignoringUntilNextLine = true;
			continue;
		}
		
		// byte parser
		
		// if a block isn't being parsed currently, look for a delimiter
		if(blockParsing < 0){
			// look for delimiter
			for(int32_t i = 0; i < sizeof(blockDelimiters)/sizeof(char); i++){
				if(byte == blockDelimiters[i]){
					blockParsing = i;
					
					// skip block open
					byteIndex++;
					
					break;
				}
			}
			
			continue;
		}
		
		// if a block is being parsed, pass the byte to the block parser function
		bool done = (*blockCharParser)(blockBuffer, byte);
		
		if(done){
			// parse block into scene
			(*blockParsers[blockParsing])(blockBuffer, scene);
			
			// reset block parsing (blockBuffer is freed by block parser when done)
			blockParsing = -1;
			
			// empty the block
			emptyBlock(blockBuffer);
		}
	} while( byte != 0 ); // end at null terminator
	
	free(fileBuffer);
	
	destroyBlock(blockBuffer);
}