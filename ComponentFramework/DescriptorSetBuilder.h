#ifndef DESCRIPTORSETBUILDER_H
#define DESCRIPTORSETBUILDER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <assert.h>
#include "CoreStructs.h"
/// A descriptor can be thought of as a pointer to a UBO or an image sampler.
/// This information is then available the pipeline.
/// However, unlike a regular pointer, it also contains additional information. 
/// (See my structure below)
/// Descriptors cannot be bound alone (unlike OpenGl)
/// they must be stored in sets - Descriptor Sets. 
/// There can be multiple Descriptor Sets. Each Descriptor Set needs to be given 
/// a unique Set Layout ID



/// Crazy idea: assemble all the raw data I'll need to create a layout, pool and the sets in one master
/// data collection. Map the binding point with the data so the reusing a binding can easily be found, 
/// see unordered_map count().

struct SingleDescriptorInfoCollection { // Holds whatever info we need in a single descriptor set.
    uint32_t binding; /// Set the binding point - this must be an unique uint so there's no duplicates. Has to be in here.
    uint32_t descriptorCount; // It can have multiple things point to it, it can point to an array of things. But let's not, because then we need to have *all* the details about it, and have it all in an array notation in the pipeline
    // like [0] diffuse map, [1] normal map, etc. But binding 1, binding 2, binding 3 etc is easier. This is always 1, but it's a variable type.
    VkDescriptorType descriptorType; // What kind of descriptor? One is a 2D sampler, another is a UBO - just for today (there's actually more), we have to specify
    VkSampler* pImmutableSamplers; // Immutable means unchangeable. 
    VkShaderStageFlags stageFlags; /// // Send it a constant, or a bunch of constants, saying which stage this'll be sent to - vertex, frag, tess, geometry, raytracing, etc...

    /// If it's a UBO use/pass these 
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize range;

    /// OR, if it's a sampler use/pass these
    VkImageLayout imageLayout;
    VkImageView imageView;
    VkSampler sampler;

    /// There must be a swapchain number of UBO data
    std::vector<BufferMemory> bufferMem;

    /// OR just the one sampler data 
    Sampler2D* pImageMem;
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
    /// is already being used. See add() methods
    std::unordered_map<uint32_t, SingleDescriptorInfoCollection> desciptorsData{};

    uint32_t numberOfDescriptors;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSet;
    uint32_t numSwapChains;

public:
    DescriptorSetBuilder(VkDevice device_);
    ~DescriptorSetBuilder();

    /// Add UBO descriptor
    DescriptorSetBuilder& add(uint32_t binding, VkDescriptorType descriptorType, // what binding point, what kind of stuff it is
        VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> bufferData); // staging point, how many, pass me the whole vector of buffer objects by pointer

    /// Add Sampler2D descriptor
    DescriptorSetBuilder& add(uint32_t binding, VkDescriptorType descriptorType, // what binding point, what type
        VkShaderStageFlags stageFlags, uint32_t count, Sampler2D* imageData); // stage flags you wanna go to, and pass a pointer to the image

    DescriptorSetInfo BuildDescriptorSet(uint32_t numSwapChains_);

    std::vector<VkDescriptorSet>& getDescriptorSet() { return descriptorSet; } // retreive it
    VkDescriptorPool getDescriptorPool() { return descriptorPool; } // get the pool
    VkDescriptorSetLayout  getDescriptorSetLayout() { return descriptorSetLayout; } // get the layout


private:
    void createLayout(); // create layout
    void createDescriptorPool(); // create pool
    void allocateDescriptorSet();  // allocate the memory
    void populateDescriptorSet(); // and populate the set

};

#endif
