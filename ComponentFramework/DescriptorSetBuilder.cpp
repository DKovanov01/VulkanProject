#include "DescriptorSetBuilder.h"
DescriptorSetBuilder::DescriptorSetBuilder(VkDevice device_) :device(device_), numberOfDescriptors(0),
descriptorPool(0), descriptorSetLayout(0), numSwapChains(0) {
    desciptorsData.clear();
}


/// I'm creating two overloaded add() methods. The first one is add a uniform buffer object descriptor
/// and the second one is to add a sampler2D descriptor to the new descriptor set.
DescriptorSetBuilder& DescriptorSetBuilder::add(uint32_t binding, VkDescriptorType descriptorType, // add function for UBO data
    VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> bufferData) {

    SingleDescriptorInfoCollection singleDescriptorInfo{}; // add/make a single data structure and populate it
    assert(desciptorsData.count(binding) == 0 && "That binding already in use"); // ask it how many are there at binding point 0, or tell me that binding point 0 is already used
    singleDescriptorInfo.binding = binding; // copy all the info here and below, comes from the list above;
    singleDescriptorInfo.descriptorCount = count; // so every time you call add, all this gets added into a descriptor set that fits into a map of sets
    singleDescriptorInfo.descriptorType = descriptorType;
    singleDescriptorInfo.pImmutableSamplers = nullptr;
    singleDescriptorInfo.stageFlags = stageFlags;
    singleDescriptorInfo.bufferMem = bufferData;
    singleDescriptorInfo.pImageMem = nullptr;

    singleDescriptorInfo.range = bufferData[0].bufferMemoryLength;
    singleDescriptorInfo.offset = 0; // don't have to do offset at 0, but would be awful if you don't
    desciptorsData[binding] = singleDescriptorInfo; /// Overloaded unordered_map function
    return *this; // why return *this?
    // 
}

DescriptorSetBuilder& DescriptorSetBuilder::add(uint32_t binding, VkDescriptorType descriptorType, // add for sampler data
    VkShaderStageFlags stageFlags, uint32_t count, Sampler2D* imageData) {

    std::vector<BufferMemory>empty; /// No such thing as a null std::vector

    SingleDescriptorInfoCollection singleDescriptorInfo{};
    assert(desciptorsData.count(binding) == 0 && "That binding already in use");
    singleDescriptorInfo.binding = binding;
    singleDescriptorInfo.descriptorCount = count;
    singleDescriptorInfo.descriptorType = descriptorType;
    singleDescriptorInfo.pImmutableSamplers = nullptr;
    singleDescriptorInfo.stageFlags = stageFlags;
    singleDescriptorInfo.bufferMem = empty;
    singleDescriptorInfo.pImageMem = imageData;

    singleDescriptorInfo.range = 0;
    singleDescriptorInfo.offset = 0;
    desciptorsData[binding] = singleDescriptorInfo; /// Overloaded unordered_map function - cool 
    return *this;
}


DescriptorSetInfo DescriptorSetBuilder::BuildDescriptorSet(uint32_t numSwapChains_) { // once we have all the data from above, we build
    numSwapChains = numSwapChains_;
    createLayout();
    createDescriptorPool();
    allocateDescriptorSet();
    populateDescriptorSet();
    DescriptorSetInfo descriptorSetInfo{};
    descriptorSetInfo.descriptorPool = descriptorPool;
    descriptorSetInfo.descriptorSetLayout = descriptorSetLayout;
    descriptorSetInfo.descriptorSet = descriptorSet;
    return descriptorSetInfo;
}


void DescriptorSetBuilder::createLayout() {  // so Scott builds a map of descriptor data

    //VkDescriptorSet descriptorSetLayout{};
    std::vector<VkDescriptorSetLayoutBinding> bindings{}; // in order to build a layout, we need a vector of all the data & bindings

    /// Transfer the layoutBinding data from the map to a vector
    for (auto& b : desciptorsData) { // Scott didn't wanna write out the variable type, since unordered maps are pairs of data with 1st being Uint32; Auto is easier;
        VkDescriptorSetLayoutBinding descriptorSetLayout{};
        descriptorSetLayout.binding = b.second.binding;
        descriptorSetLayout.descriptorCount = b.second.descriptorCount;
        descriptorSetLayout.descriptorType = b.second.descriptorType;
        descriptorSetLayout.pImmutableSamplers = nullptr;
        descriptorSetLayout.stageFlags = b.second.stageFlags;
        bindings.push_back(descriptorSetLayout);
    }
    numberOfDescriptors = static_cast<uint32_t>(desciptorsData.size());

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = numberOfDescriptors;
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) { // retreive the map by ID number;
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    ///return descriptorSetLayout; /// Return the local variable
}


void DescriptorSetBuilder::createDescriptorPool() { // next stage - create the pool
    //VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorPoolSize> poolSizes{};

    for (auto& b : desciptorsData) { // good thing we have the database, it has all the stuffs we nede. We'll loop over them and figure out how bug the pool has to be
        VkDescriptorPoolSize descriptorPoolSize{};
        descriptorPoolSize.type = b.second.descriptorType;
        descriptorPoolSize.descriptorCount = numSwapChains;
        poolSizes.push_back(descriptorPoolSize);
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = poolSizes.size();
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = numSwapChains;
    descriptorPoolInfo.flags = 0;
    /// SSF just keep this in mind
    ///descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT; // not today
    if (vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) { // and then we create the pool
        throw std::runtime_error("failed to create descriptor pool!");
    }
}



/// numSwapChains is usually going to be swapChainImages.size()
void DescriptorSetBuilder::allocateDescriptorSet() {

    descriptorSet.resize(numSwapChains);

    /// Make a numSwapChains length vector and initialize each member of that array
    /// with the value of the descriptorSetLayout 
    std::vector<VkDescriptorSetLayout> layouts(numSwapChains, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChains);
    allocInfo.pSetLayouts = layouts.data();

    /// descriptorSet.data is pointer to an array where the allocated descriptor set handles will be stored.
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSet.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void DescriptorSetBuilder::populateDescriptorSet() {

    std::vector<VkWriteDescriptorSet> descriptorWrites(numberOfDescriptors);
    std::vector<VkDescriptorBufferInfo>bufferInfoList;
    std::vector<VkDescriptorImageInfo>imageInfoList;

    /// Now populate the descriptorSet with the VRAM data handles
    for (uint32_t i = 0; i < numSwapChains; i++) { /// Loop over the number of descriptor sets
        for (auto& b : desciptorsData) { /// loop over the individual descriptors

            if (b.second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = b.second.bufferMem[0].bufferID;
                bufferInfo.offset = b.second.offset; /// Better be zero
                bufferInfo.range = b.second.bufferMem[0].bufferMemoryLength;
                bufferInfoList.push_back(bufferInfo);

            }
            else if (b.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = b.second.pImageMem->imageView;
                imageInfo.sampler = b.second.pImageMem->sampler;
                imageInfoList.push_back(imageInfo);
            }
        }

        /// While looping over the desciptorsData map - keep track of which buffer data
        /// set along with the image data sets
        int bufferIndex = 0;
        int imageIndex = 0;
        int descriptorIndex = 0;

        for (auto& b : desciptorsData) { /// loop over the individual descriptors 
            if (b.second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                descriptorWrites[descriptorIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[descriptorIndex].dstSet = descriptorSet[i];/// Destination descriptor set
                descriptorWrites[descriptorIndex].dstBinding = b.second.binding;
                descriptorWrites[descriptorIndex].dstArrayElement = 0;
                descriptorWrites[descriptorIndex].descriptorType = b.second.descriptorType;
                descriptorWrites[descriptorIndex].descriptorCount = b.second.descriptorCount;

                descriptorWrites[descriptorIndex].pBufferInfo = &bufferInfoList[bufferIndex];
                descriptorIndex++;
                bufferIndex++;


            }
            else if (b.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                descriptorWrites[descriptorIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[descriptorIndex].dstSet = descriptorSet[i];/// Destination descriptor set
                descriptorWrites[descriptorIndex].dstBinding = b.second.binding;
                descriptorWrites[descriptorIndex].dstArrayElement = 0;
                descriptorWrites[descriptorIndex].descriptorType = b.second.descriptorType;
                descriptorWrites[descriptorIndex].descriptorCount = b.second.descriptorCount;

                descriptorWrites[descriptorIndex].pImageInfo = &imageInfoList[imageIndex];
                descriptorIndex++;
                imageIndex++;

            }
        }
        /// This updates the descriptor set specified in the dstSet field of the of the descriptorWrites structures. 
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        /// Now, I'm about to start on the next descriptor set, so clear the data lists
        bufferInfoList.clear();
        imageInfoList.clear();
    }
}


DescriptorSetBuilder::~DescriptorSetBuilder() {

}




