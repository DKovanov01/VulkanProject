#include "DescriptorSetBuilder.h"

    DescriptorSetBuilder::DescriptorSetBuilder(VkDevice device_):device(device_) {}
    DescriptorSetBuilder::~DescriptorSetBuilder(){}

    DescriptorSetBuilder& DescriptorSetBuilder::add(uint32_t binding, VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> *bufferData, Sampler2D *imageData) {
       
        assert(bindings.count(binding) == 0 && "That binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;
		bindings[binding] = layoutBinding; /// Overloaded unordered_map function
        if( bufferData != nullptr){
            VkDescriptorBufferInfo uboInfo{};
            uboInfo.buffer = bufferData[0].
		return *this;  
    }

    DescriptorSetLayout DescriptorSetBuilder::createLayout(){ 
        DescriptorSetLayout descriptorSetLayout{};
        descriptorSetLayout.bindings.clear();/// Just being careful
        /// Transfer the layoutBinding data from the map to a vector
        for (auto &b : bindings) {
			descriptorSetLayout.bindings.push_back(b.second);
		}
        bindingCount = static_cast<uint32_t>(bindings.size());
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindingCount;
        layoutInfo.pBindings = descriptorSetLayout.bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return descriptorSetLayout; /// Return the local variable
    }

    void DescriptorSetBuilder::destroyLayout(DescriptorSetLayout &descriptorSetLayout){
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout.layout, nullptr);
        descriptorSetLayout.bindings.clear();
    }
   
    VkDescriptorPool DescriptorSetBuilder::createDescriptorPool(const DescriptorSetLayout &layout, uint32_t maxSets) {
        VkDescriptorPool descriptorPool;
        uint32_t numberOfDescriptors = layout.bindings.size();
        std::vector<VkDescriptorPoolSize> poolSizes(numberOfDescriptors);
        for(uint32_t i = 0; i < numberOfDescriptors; i++) {
            poolSizes[i].type = layout.bindings[i].descriptorType; 
            poolSizes[i].descriptorCount = layout.bindings[i].descriptorCount;
        }

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = 0;
         ///descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        if (vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
        return descriptorPool;
    }

    void DescriptorSetBuilder::destroyDescriptorPool(VkDescriptorPool &descriptorPool){
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

    /// numSets is usually goint to be swapChainImages.size()
    std::vector<VkDescriptorSet> DescriptorSetBuilder::allocateDescriptor(uint32_t numSets, 
        VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
        std::vector<VkDescriptorSet> descriptorSet; /// Make the local variable as usual 

        /// Make a numSets length vector and initialize each member of that array
        /// with the value of the descriptorSetLayout 
        std::vector<VkDescriptorSetLayout> layouts(numSets, descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(numSets);
        allocInfo.pSetLayouts = layouts.data();

        /// descriptorSet.data is pointer to an array where the allocated descriptor set handles will be stored.
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSet.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        /// Ready to write to the descriptorSet so zero out the writeCount
        writeCount = 0;
        return descriptorSet;
    }

    std::vector<VkWriteDescriptorSet>& DescriptorSetBuilder::addBufferInfo(std::vector<VkDescriptorSet>&descriptorSet,
             const DescriptorSetLayout layout,std::vector<BufferMemory> &bufferData ) {
        
         for (size_t i = 0; i < descriptorSet.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = bufferData[i].bufferID;
            bufferInfo.offset = 0;
            bufferInfo.range = bufferData[i].bufferMemoryLength;

            VkWriteDescriptorSet descriptorWrites{};
            descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites.dstSet = descriptorSet[i];
            descriptorWrites.dstBinding = layout.bindings[i].binding;
            descriptorWrites.dstArrayElement = 0;
            descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites.descriptorCount = 1;
            descriptorWrites.pBufferInfo = &bufferInfo;
            writes.push_back(descriptorWrites);
         }
         return writes;
    }


    std::vector<VkWriteDescriptorSet>& DescriptorSetBuilder::addImageInfo(std::vector<VkDescriptorSet>&descriptorSet,
             const DescriptorSetLayout layout,Sampler2D &imageData ) {
        
         for (size_t i = 0; i < descriptorSet.size(); i++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageData.imageView;
            imageInfo.sampler = imageData.sampler;

            VkWriteDescriptorSet descriptorWrites{};
            descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites.dstSet = descriptorSet[i];
            descriptorWrites.dstBinding = layout.bindings[i].binding;
            descriptorWrites.dstArrayElement = 0;
            descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites.descriptorCount = 1;
            descriptorWrites.pImageInfo = &imageInfo;
            writes.push_back(descriptorWrites);
         }
         return writes;
    }