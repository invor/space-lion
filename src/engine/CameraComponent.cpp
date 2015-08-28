#include "CameraComponent.h"

CameraComponentManager::CameraComponentManager()
{
}

CameraComponentManager::CameraComponentManager(uint size)
{
}

CameraComponentManager::~CameraComponentManager()
{
}

void CameraComponentManager::reallocate(uint size)
{
}

void CameraComponentManager::addComponent(Entity entity)
{
}

void CameraComponentManager::addComponent(Entity entity, float near, float far, float fovy, float aspect_ratio)
{
}

void CameraComponentManager::deleteComponent(Entity entity)
{
}

void CameraComponentManager::getIndex(Entity entity)
{
}

void CameraComponentManager::setCameraAttributes(uint index, float near, float far, float fovy, float aspect_ratio)
{
}

void CameraComponentManager::updateViewMatrix(uint index)
{
}

void CameraComponentManager::updateProjectionMatrix(uint index)
{
}