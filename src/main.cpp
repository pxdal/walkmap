// main 

#include <world.hpp>
#include <walkmap.hpp>

#include <argparse/argparse.hpp>

#include <cstdlib>
#include <cstdio>

#include <string>
#include <fstream>
#include <chrono>

void initializeArguments(argparse::ArgumentParser& parser);

int main(int argc, char** argv){
	// set stdout buffer flushed without buffering, because line buffering doesn't work in msys2 terminal
	#ifdef __MINGW64__
		setvbuf(stdout, NULL, _IONBF, 0);
	#endif
	
	// start time
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	
	// parse arguments
	// macros defined in walkmap.hpp
	argparse::ArgumentParser argParser = argparse::ArgumentParser(PROG_NAME, PROG_VERSION);
	
	initializeArguments(argParser);
	
	try {
		argParser.parse_args(argc, argv);
	} catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << argParser; // NOTE: logs help message
		
		exit(EXIT_FAILURE);
	}
	
	// get file path
	std::string path = argParser.get<std::string>("--world");
	
	printf("Parsing .world file...\n");
	
	// load world
	Scene* world = parseWorld(path.c_str());
	
	if(world == NULL){
		exit(EXIT_FAILURE);
	}
	
	// create walkmap from settings
	
	// load settings
	WalkmapSettings settings;
	
	settings.playerHeight = argParser.get<float>("--player-height");
	settings.playerRadius = argParser.get<float>("--player-radius");
	settings.stepHeight = argParser.get<float>("--player-step-height");
	settings.maxPlayerSpeed = argParser.get<float>("--player-max-speed");
	settings.heightSpeed = argParser.get<float>("--height-adjustment-speed");
	settings.generateIds = argParser.get<bool>("--generate-walkbox-ids");
	
	std::string buffer;
	std::string outPath = argParser.get<std::string>("--walkmap");
	std::ofstream out;
	
	bool generateWalkmapArg = !argParser.get<bool>("--no-walkmap");
	
	std::vector<BoundingBox*> walkmap;
	
	if(generateWalkmapArg){
		// resize all objects indiscriminately
		for(uint32_t i = 0; i < world->objects->size(); i++){
			world->objects->at(i)->scale.x += settings.playerRadius;
			world->objects->at(i)->scale.z += settings.playerRadius;
		}
		
		printf("Generating walkmap...\n");
		
		// generate walkmap
		generateWalkmap(settings, world->objects, &walkmap);
		
		printf("Writing walkmap to file...\n");
		
		// open file
		out.open(outPath);
		
		// get buffer
		walkmapToBuffer(buffer, &walkmap, settings);
		
		// pipe to file
		out << buffer;
		
		out.close();
	}
	
	bool generateWalkmapWorld = argParser.get<bool>("--generate-walkmap-world");
	bool noWalkmap = argParser.get<bool>("--no-walkmap");
	
	if(generateWalkmapWorld || noWalkmap){
		printf("Writing walkmap.world to file...\n");
		
		buffer = "";
		
		if(noWalkmap){
			walkmap.clear();
			
			// adapt walkmap into bounding boxes
			for(uint32_t i = 0; i < world->objects->size(); i++){
				Object* object = world->objects->at(i);
				
				walkmap.push_back( createBbox(object->position, glm::vec2(object->scale.x, object->scale.z) ) );
			}
		}
		
		walkmapToWorld(buffer, &walkmap, settings);
		
		out.open( outPath + ".world" );
	
		out << buffer;
	
		out.close();
	}
	
	// get time elapsed
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed = end-start;
	
	printf("Done (finished in %f seconds).\n", elapsed.count());
	
	return EXIT_SUCCESS;
}

void initializeArguments(argparse::ArgumentParser& parser){
	// description
	parser.add_description("Generate a walkmap from a .world file");
	
	// arguments
	parser.add_argument("--in", "--world")
		.help("path to a .world file (or .walkmap file, if only generating a walkmap .world) to generate walkmap for")
		.required()
		.append();
		
	parser.add_argument("--out", "--walkmap")
		.help("where to output the walkmap + walkmap.world file")
		.append();
	
	parser.add_argument("--player-height")
		.help("used to determine if objects will obstruct the player's walkable space or not.  overrides any value present in the .world file.")
		.default_value<float>(2.f)
		.scan<'g', float>();
		
	parser.add_argument("--player-radius")
		.help("used to determine the distance that bounding box edges should have from unsteppable walls.  overrides any value present in the .world file.")
		.default_value<float>(0.25f)
		.scan<'g', float>();
	
	parser.add_argument("--player-step-height")
		.help("used to determine which objects the player can step onto.  overrides any value present in the .world file.")
		.default_value<float>(0.4f)
		.scan<'g', float>();
		
	parser.add_argument("--player-max-speed")
		.help("not really used by the walkmap generator at all, but if it's not defined in the .world file you can use this to set it to something other than default.")
		.default_value<float>(2.f)
		.scan<'g', float>();
		
	parser.add_argument("--height-adjustment-speed")
		.help("not really used by the walkmap generator at all, but if it's not defined in the .world file you can use this to set it to something other than default.")
		.default_value<float>(10.f)
		.scan<'g', float>();
	
	
	parser.add_argument("--no-walkmap")
		.help("whether or not to generate a walkmap.  only generates a walkmap world if false (regardless of --generate-walkmap-world flag)")
		.default_value(false)
		.implicit_value(true);
	
	parser.add_argument("--generate-walkmap-world")
		.help("whether to generate a separate .world file representing the walkmap as a bunch of objects.")
		.default_value(false)
		.implicit_value(true);
		
	parser.add_argument("--generate-walkbox-ids")
		.help("generates ids for walkboxes if the parent object of the walkbox has an id.")
		.default_value(true)
		.implicit_value(true);
}