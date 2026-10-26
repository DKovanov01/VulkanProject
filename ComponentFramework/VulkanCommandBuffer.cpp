#include "VulkanRenderer.h"

void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &primaryCommandBuffer.commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}


void VulkanRenderer::CreateCommandBuffers() {
    primaryCommandBuffer.commandBuffers.resize(numSwapchains); // create X number for however many swap chains we have

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = primaryCommandBuffer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)primaryCommandBuffer.commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, primaryCommandBuffer.commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::RecordCommandBuffers(Recording start_stop) { // pass in the enum of whether it'll start or stop
    if (start_stop == Recording::START) { // if it's a start recording
        vkDeviceWaitIdle(device); /// This is bad // but Scott doesn't wanna do it good because that's long; make sure GPU is done drawing what it was asked to last time
        for (size_t i = 0; i < primaryCommandBuffer.commandBuffers.size(); i++) { // loop over all the buffers
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(primaryCommandBuffer.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{}; // fill out some forms
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i]; // where is it? swap chains. Loop over them
            renderPassInfo.renderArea.offset = { 0, 0 }; // starts at 0,0 - duh
            renderPassInfo.renderArea.extent = swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // set the color for something other than black? change these values
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            vkCmdBeginRenderPass(primaryCommandBuffer.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // main call to begin the render pass
        }
    }
    else if (start_stop == Recording::STOP) {
        for (size_t i = 0; i < primaryCommandBuffer.commandBuffers.size(); i++) {
            vkCmdEndRenderPass(primaryCommandBuffer.commandBuffers[i]);
            if (vkEndCommandBuffer(primaryCommandBuffer.commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
}


void VulkanRenderer::DestroyCommandBuffers() {
    vkFreeCommandBuffers(device, primaryCommandBuffer.commandPool, 1, primaryCommandBuffer.commandBuffers.data());
    vkDestroyCommandPool(device, primaryCommandBuffer.commandPool, nullptr);
}