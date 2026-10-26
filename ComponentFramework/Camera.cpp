#include "Camera.h"

bool Camera::OnCreate(VulkanRenderer* vRenderer_) {
	int width = 0, height = 0;
	float aspectRatio;

	VulkanRenderer* vRenderer = vRenderer_;

	cameraUBO = vRenderer->CreateUniformBuffers<CameraData>();

	SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.projectionMatrix = MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
	camera.projectionMatrix[5] *= -1.0f;
	camera.viewMatrix = MMath::translate(0.0f, 0.0f, -5.0f);
	//camera.viewMatrix = MMath::toMatrix4(DQMath::inverse(orientationPosition));

	vRenderer->UpdateUniformBuffer<CameraData>(camera, cameraUBO);

	return true;
}

/*void Camera::UpdateUniformBuffer(VulkanRenderer* renderer) {
    if (renderer == nullptr || cameraUBO.empty()) {
        return;
    }

    CameraData data;
    data.projectionMatrix = projectionMatrix;
    data.viewMatrix = getViewMatrix();

    renderer->UpdateUniformBuffer<CameraData>(data, cameraUBO);
}*/
