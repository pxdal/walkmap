// general purpose utility methods

#include <utils.hpp>

#include <limits>

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cfloat>
#include <algorithm>

// NOTE: Free buffer when done with it.
char* read_entire_file(const char* file)
{
	FILE* source_file = fopen(file, "rb");
	if (!source_file)
	{
		perror("poor");
		return NULL;
	}
	fseek(source_file, 0, SEEK_END);
	long src_size = ftell(source_file);
	fseek(source_file, 0, SEEK_SET);
	char* buffer = (char*) malloc(src_size + 1);
	fread(buffer, 1, src_size, source_file);
	fclose(source_file);
	buffer[src_size] = 0;
	return buffer;
}

// https://stackoverflow.com/a/32334103
bool nearly_equal(float a, float b){
	float epsilon = 128 * FLT_EPSILON;
	float abs_th = FLT_MIN;
	
  assert(std::numeric_limits<float>::epsilon() <= epsilon);
  assert(epsilon < 1.f);

  if (a == b) return true;

  auto diff = std::abs(a-b);
  auto norm = std::min((std::abs(a) + std::abs(b)), std::numeric_limits<float>::max());
  // or even faster: std::min(std::abs(a + b), std::numeric_limits<float>::max());
  // keeping this commented out until I update figures below
  return diff < std::max(abs_th, epsilon * norm);
}

// quick shorthand 
bool nearly_less_or_eq(float a, float b){
	return (a < b) || nearly_equal(a, b);
}