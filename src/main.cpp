// main 

#include <world.hpp>
#include <walkmap.hpp>

#include <cstdlib>
#include <cstdio>
#include <string>

int main(int argc, char** argv){
	// set stdout buffer flushed without buffering, because line buffering doesn't work in msys2 terminal
	#ifdef __MINGW64__
		setvbuf(stdout, NULL, _IONBF, 0);
	#endif
	
	// parse arguments
	
	// should be at least one (the world file)
	if(argc < 2){
		printf("Error: Not enough arguments, requires at least 1 (.world file path)\n");
		
		exit(EXIT_FAILURE);
	}
	
	// get file path
	const char* path = argv[1];
	
	printf("Parsing .world file...\n");
	
	// load world
	std::vector<Object*> objects;
	
	parseWorld(path, &objects);
	
	// create walkmap from settings
	
	// load settings
	WalkmapSettings settings;
	
	settings.playerHeight = argc >= 3 ? std::stof(argv[2]) : 1.5f;
	settings.playerRadius = argc >= 4 ? std::stof(argv[3]) : 0.25f;
	settings.stepHeight = argc >= 5 ? std::stof(argv[4]) : 0.1f;
	
	printf("Generating walkmap...\n");
	
	// generate walkmap
	std::vector<BoundingBox*> walkmap;
	
	generateWalkmap(settings, &objects, &walkmap);
	
	/*for(uint32_t i = 0; i < walkmap.size(); i++){
		printf("\npos: %f, %f, %f\nsize: %f, %f\n", walkmap[i]->position.x, walkmap[i]->position.y, walkmap[i]->position.z, walkmap[i]->size.x, walkmap[i]->size.y);
	}*/
	
	printf("Done.\n");
	
	return EXIT_SUCCESS;
}