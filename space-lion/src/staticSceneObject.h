#ifndef staticSceneObject_h
#define staticSceneObject_h

//openGL Math Lib
#include "sceneEntity.h"

class staticSceneObject : public sceneEntity
{
private:
	glm::quat orientation;

	bool isRendered;

	vertexGeometry *geometry;
	material *mtl;
public:
	staticSceneObject();
	~staticSceneObject();

	staticSceneObject(const int, vertexGeometry*, material*);

	void setOrientation(const glm::quat inOrientation) {orientation = inOrientation;}
	glm::quat getOrientation() {return orientation;}
};

#endif sceneEntity
