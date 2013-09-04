#ifndef staticSceneObject_h
#define staticSceneObject_h

//openGL Math Lib
#include "sceneEntity.h"
#include <memory>

class StaticSceneObject : public SceneEntity
{
private:
	bool isRendered;

	std::shared_ptr<Mesh> geometry;
	std::shared_ptr<Material> material;
public:
	StaticSceneObject() {}
	~StaticSceneObject() {}

	StaticSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation,
						std::shared_ptr<Mesh> inGeom, std::shared_ptr<Material> inMtl) : 
							SceneEntity(inId, inPosition, inOrientation), geometry(inGeom), material(inMtl) {}

	const std::shared_ptr<Mesh> getGeometry() {return geometry;}
	const std::shared_ptr<Material> getMaterial() {return material;}

	glm::mat4 computeModelMatrix() {return (glm::translate(glm::mat4(1.0),position))*(glm::mat4_cast(orientation))*(glm::scale(glm::mat4(1.0),scaling));}
};

#endif
