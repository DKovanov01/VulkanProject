#pragma once
#include "Renderer.h"
#include "VulkanRenderer.h"
#include "CoreStructs.h"
#include <DualQuat.h>
#include <MMath.h>

using namespace MATH;
using namespace MATHEX;

class Camera {
private:
    DualQuat orientationPosition;
    Matrix4 projectionMatrix;
    Matrix4 viewMatrix;

    std::vector<BufferMemory> cameraUBO;
    CameraData camera;

    VulkanRenderer* vRenderer_;// = nullptr;

public:
    Matrix4 getViewMatrix() { return MMath::toMatrix4(orientationPosition); }
    //Matrix4 getProjectionMatrix() const { return projectionMatrix; };
    bool OnCreate(VulkanRenderer* renderer);
    //void UpdateUniformBuffer(VulkanRenderer* renderer);
    const std::vector<BufferMemory> getCameraUBO() { return cameraUBO; }

    //void SetAspectRatio(float aspectRatio, VulkanRenderer* renderer);
};