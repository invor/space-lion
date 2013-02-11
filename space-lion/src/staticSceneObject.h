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

	staticSceneObject(const int, glm::vec3, vertexGeometry*, material*);

	void rotate(const glm::quat rotation);
	void setOrientation(const glm::quat inOrientation) {orientation = inOrientation;}

	glm::mat4 computeModelMatrix();
};

#endif sceneEntity
