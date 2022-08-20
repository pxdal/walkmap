// main 

#include <world.hpp>
#include <walkmap.hpp>

#include <cstdlib>
#include <cstdio>

#include <string>
#include <fstream>

int main(int argc, char** argv){
	// set stdout buffer flushed without buffering, because line buffering doesn't work in msys2 terminal
	#ifdef __MINGW64__
		setvbuf(stdout, NULL, _IONBF, 0);
	#endif
	
	// parse arguments
	
	// should be at least one (the world file)
	if(argc < 3){
		printf("Error: Not enough arguments, requires at least 2 (.world file path, out path)\n");
		
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
	
	settings.playerHeight = argc >= 4 ? std::stof(argv[3]) : 2.f;
	settings.playerRadius = argc >= 5 ? std::stof(argv[4]) : 0.5f;
	settings.stepHeight = argc >= 6 ? std::stof(argv[5]) : 0.4f;
	settings.maxPlayerSpeed = argc >= 7 ? std::stof(argv[6]) : 2.f;
	
	printf("Generating walkmap...\n");
	
	// generate walkmap
	std::vector<BoundingBox*> walkmap;
	
	generateWalkmap(settings, &objects, &walkmap);
	
	printf("Writing walkmap to file...\n");
	
	// open file
	std::ofstream out;
	out.open(argv[2]);
	
	// get buffer
	std::string buffer;
	std::string otherBuffer;
	
	walkmapToBuffer(buffer, &walkmap, settings);
	walkmapToWorld(otherBuffer, &walkmap, settings);
	
	// pipe to file
	out << buffer;
	
	out.close();
	
	out.open( (std::string(argv[2]) + ".world").c_str() );
	
	out << otherBuffer;
	
	out.close();
	
	printf("Done.\n");
	
	return EXIT_SUCCESS;
}