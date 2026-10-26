#include "VulkanRenderer.h"

PipelineInfo VulkanRenderer::CreateGraphicsPipeline(std::vector<VkDescriptorSetLayout> descriptorSetLayoutArray, const char* vertFile, const char* fragFile, // make a swap chain mumber of pipelines
    const char* tessCtrlFile, const char* tessEvalFile, const char* geomFile) { // we get vert file, frag file, 2 tesselation and 1 geometry
    // right now the descriptorsetlayout doesn't have a & or *, so we're sending a temporary local variable. Scott is recommending, for assignment 2, tomake that into 
    // a vector of them. Make 1 set, put it into the vector, see if it runs. If it does, then we can have 2+;

    PipelineInfo graphicsPipeInfo;

    std::vector<char> vertShaderCode; // just do this with a Vec in case it's many many vecs
    std::vector<char> fragShaderCode; // hold the code for any one of these thihgs
    std::vector<char> tessCtrlCode;
    std::vector<char> tessEvalCode;
    std::vector<char> geomCode;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE; // next we make all these modules. VK_NULL_HANDLE = null pointer = 0;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE; // they need handles
    VkShaderModule tessCtrlModule = VK_NULL_HANDLE; // it's called this so that we know it's a handle and it's set to 0
    VkShaderModule tessEvalModule = VK_NULL_HANDLE; // because technically we could just write null, 0, nullptr, etc. Either way, it's initialized to 0
    VkShaderModule geomModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{}; // make sure everything's set to 0 so everything is ultra clean before we start;
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{}; // need a bunch of forms that we'll fill out
    VkPipelineShaderStageCreateInfo tessCtrlStageInfo{};
    VkPipelineShaderStageCreateInfo tessEvalStageInfo{};
    VkPipelineShaderStageCreateInfo geomStageInfo{};

    if (vertFile != nullptr && fragFile != nullptr) { //like in OpenGL, read each individual module to eventually turn it into a shader.
        vertShaderCode = readFile(vertFile);
        fragShaderCode = readFile(fragFile);
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
    }

    if (tessCtrlFile != nullptr && tessEvalFile != nullptr) { // open up all the files we can possibly find
        tessCtrlCode = readFile(tessCtrlFile);
        tessEvalCode = readFile(tessEvalFile);
        tessCtrlModule = createShaderModule(tessCtrlCode);
        tessEvalModule = createShaderModule(tessEvalCode);
    }

    if (geomFile != nullptr) { // open up the file, read all the stuff we need
        geomCode = readFile(geomFile);
        geomModule = createShaderModule(geomCode);

    }

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // just fill out some government forms :p so what stage are we building? a vertex stage, duh | AKA form 18
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // set the vertex bit -> tell it that this is a vertex stage
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // remind it that it's 'main' because it could be something else; p, stands for pointer, so we need a pointer to the entry point

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // again

    tessCtrlStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessCtrlStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessCtrlStageInfo.module = tessCtrlModule;
    tessCtrlStageInfo.pName = "main"; // and again

    tessEvalStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessEvalStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessEvalStageInfo.module = tessEvalModule;
    tessEvalStageInfo.pName = "main";

    geomStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geomStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geomStageInfo.module = geomModule;
    geomStageInfo.pName = "main"; //why does it say main? because don't rename it, we'll lose our minds. It's just letting us know where the program starts"

    // go inside the VkPipelineShaderStageCreateInfo
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages; //once all the forms above are filled out, build all the shader stages | the vector holds all the stage creation information

    if (vertShaderModule != VK_NULL_HANDLE) shaderStages.push_back(vertShaderStageInfo); //shaderStages using a vector because we don't know how many shaders we're gonna have
    if (tessCtrlModule != VK_NULL_HANDLE) shaderStages.push_back(tessCtrlStageInfo); // if it's not a null pointer, then we're using it. If it is, we're not. 
    if (tessEvalModule != VK_NULL_HANDLE) shaderStages.push_back(tessEvalStageInfo); // primarily we don't know if we're using a ctessCtrl and geomStage; Others we will use.
    if (geomModule != VK_NULL_HANDLE) shaderStages.push_back(geomStageInfo); // we're pushing not a pointer but the actual data that's inside the structure;
    if (fragShaderModule != VK_NULL_HANDLE) shaderStages.push_back(fragShaderStageInfo); // so we use a vector of these characters.
    // if you have a module ----------------------> push it in
    // So at the end of it we have shader stages of *some* length (because we don't know how many there are) and store them as an array;
    // how does the machine know what data is in there? because the first item in each is always a type, so it knows what's in there by that type as per struct file
    // vertShaderStageInfo is a data structure, but we're not pushing the structure in, we're pushing the data that it holds.

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // true in glsl and in all other languages, there are 2 kinds of data coming into the pipeline - per vertex data, aka attributes, aka pos, normal, etc;
    // so the 2 lines below set up the attribute business
    auto bindingDescription = Vertex::getBindingDescription(); // binding is the location that we're binding it to/location it's coming in at.
    auto attributeDescriptions = Vertex::getAttributeDescriptions(); // attributes are per-vertex data, so every vertex has it

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; //we're stapling the pVertexBindingDescription "form" to the &bindingDescription "form"
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // data structure that'll handle what type of drawing we're doing inside the triangles. Could be dots, could be quads, could be a bunch of things - we're just doing primitive triangles
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; //here's another "government form"; 
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; //what does it do? tell it what we're drawing
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //triangles, duh. Draw unique vertices with a draw triangle list
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f; //these don't have to be 0.0; Don't change that though, that's stupid.
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width); //in OpenGL it was -1 to 1, but Vulkan is 0 to 1; Why? To piss Scott off, specifically
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{}; //snip snip | that's just what part of hte screen are we looking at
    scissor.offset = { 0, 0 }; // in this case, the whole screen. But it could be a small part of the screen or split-screen, etc.
    scissor.extent = swapChainExtent;

    // structure that holds the viewport and scissor information. 
    VkPipelineViewportStateCreateInfo viewportState{}; //now we fill out another form. Staple some more shit together and get a viewport working.
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; //how many can you have? More than one. But don't.
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1; //how many can you have? More than one. But don't.
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{}; //our monitor's got a buncha dots, rasterizer will tell each dot on the monitor what color it is
    // from the memory we have. It doesn't have to be the same size as how many dots we have on screen, but for it to match, we gotta tell it to rasterize
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //use all dots inside a polygon if there's a polygon | we actually take the polygons and fill them in
    rasterizer.lineWidth = 1.0f; //how fat is the line? 1 for now. Don't touch.
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //if it's pointing the wrong way, don't rasterize. Same as if it's pointing the wrong way, don't draw it. called backface culling. Turn that one bit on, saying this guy's going that way, the other guy is going the other way.
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //which way does the stuff draw? this way. Just because. always counter-clockwise, nobody knows why. 
    rasterizer.depthBiasEnable = VK_FALSE; //is how to blend multiple data sets for 1 pixel; make size of memory buffer the size of dots on screen for convenience

    VkPipelineMultisampleStateCreateInfo multisampling{}; //how about multisampling?
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; //no
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{}; //regions I wanna draw into and ones I don't
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE; //no

    // if you turn on the color blending, then you have to actually fill in the form below. But we don't, so we don't. 
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; //do I wanna make a colorBlendAttachment? Do you wanna draw the color you got or blend the old one in with some math?
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; //no

    // originally this was used for transparency, but there are better ways of doing this now. See-throuhg objects is one of the purposes
    // but there are a whole lot of effects we can achieve by taking existing colors nearby and doing some algebra to blend them rather than replacing an existing color with another
    // if you're that deep into it though, then you're probably an artist who can write vulkan code. That's rare. 
    VkPipelineColorBlendStateCreateInfo colorBlending{}; //above was the attachment form, this is the actual blending form
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; //all 0 = just don't, we're not doing this right now, go figure it out later
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange range{}; //do we want a pushConst? 
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //where do you want it to be sent? give it specifically to the vertex
    range.offset = 0; // where does it start? At the beginning.
    range.size = sizeof(ModelMatrixPushConst); //yes; because the model matrix can be done in it and transferred through it. How big is the range? Big enough. It starts at 0
    //doesn't have to start at 0, but should, don't break it. 

    // pipelineLayoutINfo - what does the pipeline look like?
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{}; //finally: staple all the forms onto this thing
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // what type of pipeline is it?
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutArray.size(); // set layout count
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutArray.data(); // oop, assignment 2; DescriptorSetLayout isn't a descriptor set, it's a map of the descriptor set. 
    // how many descriptor sets are there? currently 1 - that's the line above; But what if we had more than 1? Like camera and lights and mario? Then we'd have 2.
    // so the line above %descriptorSetLayout is getting the address of that guy. Take a deeper look at it, it passes a singular descriptor set. 
    // What we're gonna need to do is pass in probably a vector of descriptorSetLayout(s). Could be 1, could be many, depends on how many we want to call in. 
    // Going from 1 to 2 is hard, but then we can do many because we get it. 
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range; // 
    //pipelineLayoutInfo.flags = VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &graphicsPipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    } //this creates the pipelineLayout, which is a map of what the pipeline will look like. So this is the folder with all the forms stapled in it.



    VkGraphicsPipelineCreateInfo pipelineInfo{}; //we say this pipeline contains EVERYTHING we've been filling out
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; // what type are we? VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, duh
    pipelineInfo.stageCount = shaderStages.size(); // That's the vector. We ask it how big it is, it says 2
    pipelineInfo.pStages = shaderStages.data(); // then what we want is a pointer to the data that is inside the 2-long array above. When you do .data, that brings you back the pointer to the array that holds all that information
    // and Vulkan always puts a p. in front of something to say it wants/expects a pointer to it. 
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = graphicsPipeInfo.pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    } //it either works or it doesn't; That's the only Vulkan call we have to make.


    if (fragShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);
    if (tessCtrlModule) vkDestroyShaderModule(device, tessCtrlModule, nullptr);
    if (tessEvalModule) vkDestroyShaderModule(device, tessEvalModule, nullptr);
    if (geomModule) vkDestroyShaderModule(device, geomModule, nullptr);

    return graphicsPipeInfo;
}

void VulkanRenderer::DestroyPipeline(PipelineInfo pipelineInfo) {
    vkDestroyPipelineLayout(device, pipelineInfo.pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipelineInfo.pipeline, nullptr);
}

std::vector<char> VulkanRenderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary); // ifstream reads files :) In our language, there's 2 ways of doing files - we're currently looking at c++, but then there's also C
    // so since we're in c++, we gotta give it a string. Why would you use a const character* when you could make it way uglier and make it a string? C++, bleh.
    // std::ios::ate means we want to start writing or reading at the end of the file, so we can get the size of the file. std::ios::binary means we want to write or read the file as binary data, not text.
    if (!file.is_open()) { // hey file, are you open? We want to do things.
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg(); //tellg = tell get, returns the current get read pointer position, so we can get the size of the file (since we opened it at the end). 
    //We cast it to a size_t.
    std::vector<char> buffer(fileSize); // make a vector of chars, and make it the size of the file. So we can read the whole file into this buffer.
    file.seekg(0); // seekg stands for seek get, so we can go to the beginning of the file (0).
    file.read(buffer.data(), fileSize); // read the file into the buffer. buffer.data() returns a pointer to the underlying array of the vector, so we can read the whole file into this buffer.
    file.close();
    return buffer;
}
