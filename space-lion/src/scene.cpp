#include "scene.h"


scene::scene(void)
{
}


scene::~scene(void)
{
}

void scene::render()
{
}

vertexGeometry* scene::createVertexGeometry()
{
	//	check list of vertexBufferObjects for default box object(id=0)
	for(std::list<vertexGeometry>::iterator i = vboList.begin(); i != vboList.end(); ++i)
	{
		if((i->getId())==0){
			return &*i;
		}
	}

	//	if default box not already in list, continue here
	vertex3 *vertexArray = new vertex3[8];
	GLubyte *indexArray = new GLubyte[10];

	vboList.push_back(vertexGeometry(0));
	std::list<vertexGeometry>::iterator lastElement = --(vboList.end());
	lastElement->bufferDataFromArray(vertexArray,indexArray);
}

material* scene::createMaterial()
{
	
}