#ifndef staticSceneObject_h
#define staticSceneObject_h

//openGL Math Lib
#include "sceneEntity.h"

class staticSceneObject : public sceneEntity
{
private:
	bool isRendered;

	vertexGeometry *geometry;
	material *mtl;
public:
	staticSceneObject();
	~staticSceneObject();

	staticSceneObject(const int, glm::vec3, vertexGeometry*, material*);

	vertexGeometry* getGeometry() {return geometry;}
	material* getMaterial() {return mtl;}

	glm::mat4 computeModelMatrix() {return (glm::translate(glm::mat4(1.0),position))*(glm::mat4_cast(orientation));}
};

#endif
