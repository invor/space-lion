#ifndef renderParser_h
#define renderParser_h

#include "material.h"
#include <fstream>

class renderParser
{
public:
	renderParser();
	~renderParser();

	bool parseMaterial(const char* const,materialInfo&);
};

#endif
