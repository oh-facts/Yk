#include <renderer/ykr_common.h>
#include <renderer/ykr_debug_util.h>
#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include <cgltf/cgltf_write.h>

VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = 0;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    info.samples = VK_SAMPLE_COUNT_1_BIT;

    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usage_flags;

    return info;
}

VkImageViewCreateInfo image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = 0;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspect_flags;

    return info;
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size)
{

    VkImageBlit2 blit_reg = {};
    blit_reg.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

    //srcOffset[0] is top left corner. [1] is bottom right. Over here we are specifying the extent of the copy, which is whole image.
    blit_reg.srcOffsets[1].x = src_size.width;
    blit_reg.srcOffsets[1].y = src_size.height;
    blit_reg.srcOffsets[1].z = 1;

    blit_reg.dstOffsets[1].x = dst_size.width;
    blit_reg.dstOffsets[1].y = dst_size.height;
    blit_reg.dstOffsets[1].z = 1;

    blit_reg.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_reg.srcSubresource.baseArrayLayer = 0;
    blit_reg.srcSubresource.layerCount = 1;
    blit_reg.srcSubresource.mipLevel = 0;

    blit_reg.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_reg.dstSubresource.baseArrayLayer = 0;
    blit_reg.dstSubresource.layerCount = 1;
    blit_reg.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blit_info = {};
    blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    blit_info.dstImage = dst;
    blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blit_info.srcImage = src;
    blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blit_info.filter = VK_FILTER_LINEAR;
    blit_info.regionCount = 1;
    blit_info.pRegions = &blit_reg;

    vkCmdBlitImage2(cmd, &blit_info);
}

YkBuffer ykr_create_buffer(VmaAllocator allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
{
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = alloc_size;
    info.usage = usage;
    
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = memory_usage;
    alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    YkBuffer out = {};

    VkResultAssert(vmaCreateBuffer(allocator, &info, &alloc_info, &out.buffer, &out.alloc, &out.info), "Vma buffer creation")

    return out;
}

void ykr_destroy_buffer(VmaAllocator allocator, const YkBuffer* buffer)
{
    vmaDestroyBuffer(allocator, buffer->buffer, buffer->alloc);
}

void ykr_imm_submit(VkDevice device, VkCommandBuffer cmd, VkFence fence, void (*fn)(VkCommandBuffer, void *), void* data, VkQueue queue)
{
    vkResetFences(device, 1, &fence);
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &info);
    fn(cmd, data);
    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo buffer_submit_info = {};
    buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    buffer_submit_info.commandBuffer = cmd;
    
    VkSubmitInfo2 submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.pCommandBufferInfos = &buffer_submit_info;
    submit_info.commandBufferInfoCount = 1;

    vkQueueSubmit2(queue, 1, &submit_info, fence);    
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

//ToDo(facts): Use the memory arena for all allocations. Submit, then free. Also, free the buffer you created when uploading mesh. Try to get a cube running. Then try something
//             better.

#include <renderer/yk_renderer.h>
mesh_asset* yk_load_mesh(YkRenderer* renderer, const char* filepath, void* memory, size_t size)
{
    mesh_asset* out = (mesh_asset*)malloc(sizeof(mesh_asset));

    YkMemoryArena index_arena;
    yk_memory_arena_innit(&index_arena, size / 2, memory);

    YkMemoryArena vertex_arena;
    yk_memory_arena_innit(&vertex_arena, size / 2, (u8*)memory + size/2);

    size_t index_num = 0;
    size_t vertex_num = 0;

    cgltf_options options = {};
    cgltf_data* data = NULL;

    u32* indices = (u32*)index_arena.base;
    YkVertex* vertices = (YkVertex*)vertex_arena.base;

    if (cgltf_parse_file(&options, filepath, &data) == cgltf_result_success) 
    {

        if (cgltf_load_buffers(&options, data, filepath) != cgltf_result_success) 
        {
            // handle error
        }


        for (u32 i = 0, mesh_index = 0; i < data->meshes_count; i++) 
        {
            cgltf_mesh mesh = data->meshes[i];

            mesh_asset new_mesh = {};
            new_mesh.name = mesh.name;
            new_mesh.num_surfaces = 0;
            new_mesh.surfaces = (geo_surface*)malloc(sizeof(geo_surface));

            for (u32 j = 0; j < data->meshes[i].primitives_count; j++) 
            {
                cgltf_primitive* p = &mesh.primitives[j];
                if (p->type != cgltf_primitive_type_triangles) continue;

                cgltf_accessor* index_attrib = data->meshes[i].primitives[j].indices;
                
                geo_surface surface;
                surface.start = index_num;
                surface.count = index_attrib->count;
                      
                if (index_attrib->component_type == cgltf_component_type_r_16u) 
                {
                    cgltf_size index;
                    for (size_t k = 0; k < index_attrib->count; ++k) 
                    {
                        index = cgltf_accessor_read_index(index_attrib, k);
                        indices[k] = index;
                        //yk_memory_arena_push(&index_arena, sizeof(cgltf_size), &index);
                        index_num++;
                        
                    }
                }

                for (u32 k = 0; k < p->attributes_count; k++) 
                {
                    if (p->attributes[k].type == cgltf_attribute_type_position)  // POSITION
                    {
                        cgltf_accessor* attribute = p->attributes[k].data;

                        if ((attribute->component_type == cgltf_component_type_r_32f) && (attribute->type == cgltf_type_vec3))
                        {
                            for (size_t k = 0; k < attribute->count; ++k) 
                            {
                                float* values = (float*)attribute->buffer_view->buffer->data + attribute->offset / sizeof(float) + k * 3;
                                vertices[k].pos.x = values[0];
                                vertices[k].pos.y = values[1];
                                vertices[k].pos.z = values[2];
                                

                               // yk_memory_arena_push(&vertex_arena, sizeof(YkVertex), values);
                                vertex_num++;
                              
                            }
                        }
                    }
                                                      
                    
                    if(p->attributes[k].type == cgltf_attribute_type_normal)    //NORMAL
                    {
                        cgltf_accessor* attribute = p->attributes[k].data;
                        
                        for (size_t k = 0; k < attribute->count; ++k)
                        {
                            float* values = (float*)attribute->buffer_view->buffer->data + attribute->offset / sizeof(float) + k * 3;
                            vertices[k].normal.x = values[0];
                            vertices[k].normal.y = values[1];
                            vertices[k].normal.z = values[2];
                        }

                    }
                    

                  
                }

                for (size_t k = 0; k < vertex_num; k++)
                {
                    vertices[k].color = v4{ vertices[k].normal.x, vertices[k].normal.y , vertices[k].normal.z ,  1 };
                    //vertices[k].color = v4{ 1,0,0 , 1 };
                }

                new_mesh.surfaces[j] = surface;
                new_mesh.num_surfaces ++;
            }

            new_mesh.buffer = ykr_upload_mesh(renderer, vertices, vertex_num, indices, index_num);
            out[0] = new_mesh;

        }
       
       

    }
    return out;
   /*
    for (size_t i = 0; i < index_num; i++)
    {
        printf("index %zu: %zu\n", i, indices[i]);
    }
   
    for (size_t i = 0; i < vertex_num; i++)
    {
        ykm_print_v3(vertices[i].pos);
    }
   */
}
