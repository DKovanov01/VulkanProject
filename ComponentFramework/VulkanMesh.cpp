#include "VulkanRenderer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

IndexedVertexBuffer VulkanRenderer::LoadModelIndexed(const char* filename) {
    IndexedVertexBuffer indexedVertexBuffer{}; // clear the thing, make sure it's all set to 0
    std::vector<Vertex> vertices; // great for Engine 5 class, because memory allocation :D
    std::vector<uint32_t> indices;

    tinyobj::attrib_t attrib; // use TinyOBJ to read the thing below
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) { // this thing
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices; // Scott says this is bizzare; It's an unordered map that contains a vertex;
    // we claim that a vertex is a position, a normal and texture coord all combined -> look at Vertex definition
    // remind to self what an unordered map is; It can't have duplicates though.

    for (const auto& shape : shapes) { // exactly the code we used in OpenGL
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{}; // make a blank Vertex - which is, remember, a pos, normal and uv coord - but blank

            vertex.pos = { // position; The way tinyobj does it, it doesn't break stuff into structures, it writes things as a singular array
                attrib.vertices[3 * index.vertex_index + 0], // so it goes x,y,z,x,y,z,x,y,z - structure(x,y,z), structure(x,y,z), structure(x,y,z)
                attrib.vertices[3 * index.vertex_index + 1], // those ^ are identical in memory;
                attrib.vertices[3 * index.vertex_index + 2] // they're 0,1,2, but we're naming them x,y,z because that's easier to comprehend
            };

            vertex.normal = { // normal
               attrib.normals[3 * index.normal_index + 0],
               attrib.normals[3 * index.normal_index + 1],
               attrib.normals[3 * index.normal_index + 2]
            };

            vertex.texCoord = { // UV
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };


            if (uniqueVertices.count(vertex) == 0) { // == is overloaded so a vertex can be used as a key in the unordered map;
                // we ask 'does this vertex/thing already exist inside unique vertices? that's the 0-1 Count thing from Unordered map, essentially;
                // does it through a hash lookup, more notes in notepad

                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size()); // this is not an array, it lies to us and tells us it is
                // the line above says "take the unordered map, store the thing on the left side to the left of =, right side to the right of =
                // that is a pair of things stored

                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
    CreateVertexBuffer(indexedVertexBuffer, vertices);
    CreateIndexBuffer(indexedVertexBuffer, indices);
    return indexedVertexBuffer;
}


void VulkanRenderer::CreateVertexBuffer(IndexedVertexBuffer& indexedBufferMemory, const std::vector<Vertex>& vertices) { // create a VertexBuffer AKA create/take a memory chunk to hold vertices
    indexedBufferMemory.vertBufferLength = vertices.size(); // so a buffer of 6 items Scott had in the struct; Put in all them details in here
    VkDeviceSize bufferSize = indexedBufferMemory.vertBufferLength * sizeof(Vertex); // then ask the actual memory unit to generate the memory for us
    /// Scott says we're invoking amazon; To get the stuff fromt he GPU to CPU, we take stuff out of the loading dock and get it into the truck
    /// Scott didn't really like to us about finding memory in the GPU, the only memory address we can get is a loading dock
    /// Inside the GPU there's a lil' region where we can unload all the data from the CPU, and then magically it'll be put away where 
    /// the GPU thinks it should be. 

    BufferMemory stagingBuffer; // so we build the loading dock, call it the stagingBuffer. How big should it be? Big enough to hold all the vertices
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // build a loading dock from which we'll transfer allt he stuff into the internals of hte GPU which we normally can't access
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

    void* data; // create a void* - which means I don't care what it is;
    vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, bufferSize, 0, &data); // call this guy; It'll ask it for a bunch of stuff + &data which is address
    // &data means give me the address of whatever that data is, data is the void*, so we're doing a pointer-to-a-pointer thing; Gimme the address of this thing
    // that hass the address of what we use; Give it the address of the loading dock.
    memcpy(data, vertices.data(), bufferSize); // that's as C as it gets - not even C++; Takes the whole chunk of memory and, as fast as it can,mdrops it in.
    // copies all of the data that's in vertices - how much? BufferSize much - and unloads it
    vkUnmapMemory(device, stagingBuffer.bufferMemoryID); // then forget about everything - there's no data and no loading dock

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // then create a new buffer big enough to hold all the stuff that we just did above - big enough to hold all the vertices
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexedBufferMemory.vertBufferID, indexedBufferMemory.vertBufferMemoryID); // we don't need to know anything about the rest of the variables

    CopyBuffer(stagingBuffer.bufferID, indexedBufferMemory.vertBufferID, bufferSize); // copy all of the material that's on the loading dock,
    // transfer it to the internal on the GPU of the buffer region we just created

    vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr); // once everything is transferred from the loading dock, blow it up
    vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr); // and then free the memory - clean up what we've blown up so the space is free
}

void VulkanRenderer::CreateIndexBuffer(IndexedVertexBuffer& indexedBufferMemory, const std::vector<uint32_t>& indices) { // and then do all the same things as above here :)
    indexedBufferMemory.indexBufferLength = indices.size();
    VkDeviceSize bufferSize = indexedBufferMemory.indexBufferLength * sizeof(uint32_t);
    BufferMemory stagingBuffer{};

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

    void* data;
    vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(device, stagingBuffer.bufferMemoryID);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexedBufferMemory.indexBufferID, indexedBufferMemory.indexBufferMemoryID);

    CopyBuffer(stagingBuffer.bufferID, indexedBufferMemory.indexBufferID, bufferSize);

    vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr);
    vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr);
}

void VulkanRenderer::BindMesh(IndexedVertexBuffer mesh) {
    VkBuffer vertexBuffers[] = { mesh.vertBufferID };
    VkDeviceSize offsets[] = { 0 };
    for (uint32_t i = 0; i < getNumSwapchains(); i++) { // once again, loop over all the swap chains
        vkCmdBindVertexBuffers(primaryCommandBuffer.commandBuffers[i], 0, 1, vertexBuffers, offsets); // 
        vkCmdBindIndexBuffer(primaryCommandBuffer.commandBuffers[i], mesh.indexBufferID, 0, VK_INDEX_TYPE_UINT32);
    }
}

void VulkanRenderer::DrawIndexed(IndexedVertexBuffer mesh) {
    for (uint32_t i = 0; i < numSwapchains; i++) {
        vkCmdDrawIndexed(primaryCommandBuffer.commandBuffers[i], static_cast<uint32_t>(mesh.indexBufferLength), 1, 0, 0, 0);
    }
}

void VulkanRenderer::DestroyIndexedMesh(IndexedVertexBuffer mesh) {
    vkDestroyBuffer(device, mesh.vertBufferID, nullptr);
    vkFreeMemory(device, mesh.vertBufferMemoryID, nullptr);
    vkDestroyBuffer(device, mesh.indexBufferID, nullptr);
    vkFreeMemory(device, mesh.indexBufferMemoryID, nullptr);
}