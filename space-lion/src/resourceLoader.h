#ifndef resourceLoader_h
#define resourceLoader_h

#include <fstream>
#include "material.h"
#include "vertexGeometry.h"

class resourceLoader
{
public:
	resourceLoader();
	~resourceLoader();

	bool parseMaterial(const char* const materialPath, materialInfo& inOutMtlInfo);

	bool loadFbxGeometry(const char* const path, vertexGeometry* goemPtr);
};

#endif
