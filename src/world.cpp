// world parser
#include <world.hpp>
#include <utils.hpp>

#include <cctype>

// world

// parser settings
const char commentDelimiter = '#';
const char parameterDelimiter = ',';
const char blockOpen = '[';
const char blockClose = ']';
const char objectBlockDelimiter = '$';

bool objectBlockCharParser(void** block, char byte){
	// cast pointer to block type
	ObjectBlock* objectBlock = (ObjectBlock*)(*block);
	
	// ensure that block exists, create it if it doesn't
	if(*block == NULL){
		objectBlock = allocateMemoryForType<ObjectBlock>();
		
		// construct members
		objectBlock->floats = new std::vector<float>();
		objectBlock->strings = new std::vector<std::string>();
		objectBlock->parameterIndex = 0;
		objectBlock->parameterBuffer= new std::string();
		
		// point block to created textureBlock
		*block = (void*)objectBlock;
		
		// because this should be created on the first byte, we can safely exit here to avoid the open bracket being included
		return false;
	}
	
	// parse char (if it's not the delimiter or block close)
	if(byte != parameterDelimiter && byte != blockClose){
		// push to parameter buffer
		objectBlock->parameterBuffer->push_back(byte);
	} else {
		// flush parameter buffer
		
		// check what we're parsing
		if(objectBlock->parameterIndex < ObjectBlock::numFloats){
			// if parsing floats, convert the buffer to a float and push to floats
			float floatParameter = std::stof(*objectBlock->parameterBuffer);
			
			// push to floats buffer
			objectBlock->floats->push_back(floatParameter);
			
			// reset parameterBuffer
			objectBlock->parameterBuffer->erase(0, std::string::npos);
		
			// increment parameter index
			objectBlock->parameterIndex++;
		} else if(objectBlock->parameterIndex < ObjectBlock::numFloats+ObjectBlock::numStrings){
			// if parsing strings, just push the string to strings
			objectBlock->strings->push_back(*objectBlock->parameterBuffer);
			
			// reset parameterBuffer
			objectBlock->parameterBuffer->erase(0, std::string::npos);
			
			// check if this is the end
			if(objectBlock->parameterIndex+1 == ObjectBlock::numFloats+ObjectBlock::numStrings || byte == blockClose){
				// last argument reached, return true to indicate that we're done parsing
				return true;
			}
			
			// increment parameter index
			objectBlock->parameterIndex++;
		}
	}
	
	return false;
}

void objectBlockToScene(void** block, std::vector<Object>* objects){
	// cast pointer to block type
	ObjectBlock* objectBlock = (ObjectBlock*)(*block);
	
	// create new objects
	Object object;
	
	// load some float values
	for(uint32_t i = 0; i < objectBlock->floats->size(); i++){
		(&object.position[0])[i] = objectBlock->floats->at(i);
	}
	
	// push to objects
	objects->push_back(object);
	
	// delete contents of block
	// remove the parameter buffer
	delete objectBlock->parameterBuffer;
	
	// delete the parameters objects
	delete objectBlock->floats;
	delete objectBlock->strings;
	
	// free memory
	free(objectBlock);
	
	*block = NULL;
}

// parse world
void parseWorld(const char* file, std::vector<Object>* objects){
	// file buffer
	char* fileBuffer = read_entire_file(file);
	
	if(fileBuffer == NULL){
		printf("Invalid path for world %s\n", file);
		return;
	}
	
	// settings
	char blockDelimiters[] = {objectBlockDelimiter};
	
	// control states
	bool parsingBlock = false;
	bool ignoringUntilNextLine = false;
	int32_t blockParsing = -1;
	void* blockBuffer = NULL;
	
	// pointers to char parsers
	// first param = pointer to block (void* here to be valid for all pointers)
	// second param = char to parse
	// returns false if the block hasn't finished parsing and true if it has
	bool (*charParsers[1])(void**,char) {objectBlockCharParser};
	void (*blockParsers[1])(void**,std::vector<Object>*) {objectBlockToScene};
	
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
					break;
				}
			}
			
			continue;
		}
		
		// if a block is being parsed, pass the byte to the block parser function
		bool done = (*charParsers[blockParsing])(&blockBuffer, byte);
		
		if(done){
			// parse block into scene
			(*blockParsers[blockParsing])(&blockBuffer, objects);
			
			// reset block parsing (blockBuffer is freed by block parser when done)
			blockParsing = -1;
			blockBuffer = NULL;
		}
	} while( byte != 0 ); // end at null terminator
	
	free(fileBuffer);
}