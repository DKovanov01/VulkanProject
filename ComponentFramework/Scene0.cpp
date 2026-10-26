#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene0.h"
#include <MMath.h>
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"

Scene0::Scene0(Renderer *renderer_): 
	Scene(nullptr),renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0::~Scene0() {
}

bool Scene0::OnCreate() {
	switch (renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(vRenderer->getPhysicalDevice(), &deviceProperties); // !!!! possibly make this a Singleton !!!!

		uint32_t maxPushConstantSize = deviceProperties.limits.maxPushConstantsSize;

		std::cout << "Max Push Constant Size: " << maxPushConstantSize << " bytes" << std::endl;
		
		camera = new Camera();
		camera->OnCreate(vRenderer);

		lightsUBO = vRenderer->CreateUniformBuffers<LightsData>(); // data about lights | Make a UBO for each member of the chain

		lights.diffuse[0] = Vec4(0.0, 0.0, 0.9, 0.0);
		lights.specular[0] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.ambient = Vec4(0.1, 0.1, 0.1, 0.0);
		lights.pos[0] = Vec4(-2.0f, 0.0f, -5.0f, 0.0f);
		lights.numLights = 1;
		lights.diffuse[1] = Vec4(0.9, 0.0, 0.0, 0.0);
		lights.specular[1] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.pos[1] = Vec4(2.0f, 0.0f, -5.0f, 0.0f);
		lights.numLights = 2;
		lights.diffuse[2] = Vec4(0.0, 0.9, 0.0, 0.0);
		lights.specular[2] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.pos[2] = Vec4(0.0f, 2.0f, -5.0f, 0.0f);
		lights.numLights = 3;

		vRenderer->UpdateUniformBuffer<LightsData>(lights, lightsUBO); // just write some data into the uniformBuffer | what kind/ a LightsData kind; Then pass it the vector & lights data
		// how does it know it nseeds lights and lightsUBO? Because it's a templated function
		//vRenderer->UpdateUniformBuffer<CameraData>(camera, cameraUBO); // same

		mariosPants = vRenderer->Create2DTextureImage("./textures/mario_fire.png");
		mariosMesh = vRenderer->LoadModelIndexed("./meshes/Mario.obj");

		{ // set 0 
			DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice()); // get the device from the renderer, and pass it to the descriptorSetBuilder constructor
			descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getCameraUBO()); // add a descriptor to the builder, binding point 0, type uniform buffer, stage flags vertex shader, count 1, and pass the camera UBO
			descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO); // add another descriptor to the builder, binding point 1, type uniform buffer, stage flags vertex shader and fragment shader, count 1, and pass the lights UBO
			cameraDescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains()); // build the descriptor set, and pass the number of swapchains to the builder
		}

		{ // set 1
			DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice()); // Same thing as before, this time for Mario's descriptor set and not the camera.
			descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &mariosPants); // binding point 0, type combined image sampler, stage flags fragment shader, count 1, and pass the address of Mario's pants texture
			mariosDescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains()); // 
		}

		mariosDescriptorSetLayoutArray.push_back(cameraDescriptorSetInfo.descriptorSetLayout); // push back the descriptor set layout for the camera's descriptor set into the array of descriptor set layouts
		mariosDescriptorSetLayoutArray.push_back(mariosDescriptorSetInfo.descriptorSetLayout); // Same as above. Order matters here due to them being explicitly sets 0 & 1, that is how they should go into the shader 
		// if the order is reversed, Mario's textures get fed into lights/camera data, which doesn't work

		pipelineInfo = vRenderer->CreateGraphicsPipeline(mariosDescriptorSetLayoutArray, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv"); // need to recompile these for assignment 1

	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
			float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
			///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
			if(renderer->getRendererType() == RendererType::VULKAN){
				dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
			}
			break;
		}
	
}
void Scene0::Update(const float deltaTime) {
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;
	mariosModelMatrix = MMath::rotate(elapsedTime * 90.0f, Vec3(0.0f, 1.0f, 0.0f)) ;
	luigisModelMatrix =  MMath::translate(2.0, 0.0f,0.0f) * MMath::rotate(elapsedTime * 90.0f, Vec3(1.0f, 0.0f, 0.0f)) ;
}

void Scene0::Render() const {
		switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		
		
		vRenderer->RecordCommandBuffers(Recording::START); // tell the commandBuffers (plural, keep that in mind) to start recording; Then send it a bunch of commands
		vRenderer->BindMesh(mariosMesh); // go bind the mesh
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, cameraDescriptorSetInfo.descriptorSet, 0); // next go bind a descriptor set(s) to the pipeline layout; This is a vector of descriptor sets, so we can bind more than 1 at a time
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, mariosDescriptorSetInfo.descriptorSet, 1);
		vRenderer->BindPipeline(pipelineInfo.pipeline); // go bind the pipeline
		vRenderer->SetPushConstant(pipelineInfo, mariosModelMatrix); // Set a pushConstant, so we give it a pipelineInfo, and then we pass it what we're drawing - Mario's model matrix
		vRenderer->DrawIndexed(mariosMesh); // Draw the mesh
		vRenderer->RecordCommandBuffers(Recording::STOP); // Then turn the recorder off and submit it
		vRenderer->Render();
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);
		
		break;
	}
}


void Scene0::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyCommandBuffers();
		vRenderer->DestroyPipeline(pipelineInfo);
		vRenderer->DestroyDescriptorSet(mariosDescriptorSetInfo);
		vRenderer->DestroyDescriptorSet(cameraDescriptorSetInfo);
		vRenderer->DestroyUBO(lightsUBO);
		vRenderer->DestroyUBO(camera->getCameraUBO()); // dis
		
		vRenderer->DestroySampler2D(mariosPants);
		vRenderer->DestroyIndexedMesh(mariosMesh);
		
		
		}

	
}

