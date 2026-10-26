#ifndef DESCRIPTORSETBUILDER_H
#define DESCRIPTORSETBUILDER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <assert.h>
#include "CoreStructs.h"

/// I need the layout handle but later I need the data inn the binding list
/// so I'll bundle the in a struct
struct DescriptorSetLayout {
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSetLayoutBinding> bindings{};
};

class DescriptorSetBuilder {
    /// C11 precautions 
    DescriptorSetBuilder(const DescriptorSetBuilder&) = delete;  /// Copy constructor
    DescriptorSetBuilder(DescriptorSetBuilder&&) = delete;       /// Move constructor
    DescriptorSetBuilder& operator = (const DescriptorSetBuilder&) = delete; /// Copy operator
    DescriptorSetBuilder& operator = (DescriptorSetBuilder&&) = delete;      /// Move operator
private:
    VkDevice device;
    /// I'm using a map here because it can detect if the binding location 
    /// is already being used. See addBinding
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    std::vector<VkWriteDescriptorSet> writes{};
    uint32_t writeCount;
    uint32_t poolMaxSets;
    uint32_t numSets;
    uint32_t bindingCount;

    

public:
    DescriptorSetBuilder(VkDevice device_);
    ~DescriptorSetBuilder();

     DescriptorSetBuilder& add(uint32_t binding, VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> *bufferData, Sampler2D *imageData);
    
    DescriptorSetLayout createLayout();
    void destroyLayout(DescriptorSetLayout &layout);

    /// maxSets is usually going to be numberSwapChainImages
    VkDescriptorPool createDescriptorPool(const DescriptorSetLayout &layout, uint32_t maxSets);  
    void destroyDescriptorPool(VkDescriptorPool &descriptorPool);

    /// 
     std::vector<VkDescriptorSet> allocateDescriptor(const uint32_t numSets, 
                    const VkDescriptorSetLayout descriptorSetLayout, 
                    const VkDescriptorPool descriptorPool); 


     std::vector<VkWriteDescriptorSet>& addBufferInfo(std::vector<VkDescriptorSet>&descriptorSet,
                            DescriptorSetLayout layout,
                            std::vector<BufferMemory> &bufferData);

     std::vector<VkWriteDescriptorSet>& addImageInfo(std::vector<VkDescriptorSet>&descriptorSet,
                            DescriptorSetLayout layout,
                            Sampler2D &imageData);



};



#endif
