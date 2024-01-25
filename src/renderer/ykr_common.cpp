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
#include <vector>
//dear lawd, pwease forgive me
u32 mesh_index = 0;

std::vector<u32> indices;
std::vector<YkVertex> vertices = { };

mesh_asset* out = 0;
YkRenderer* _renderer = {};

constexpr b8 debug_color = true;

void traverse_node(cgltf_node* _node)
{
    
    if (_node->mesh)
    {
            
        cgltf_mesh* mesh = _node->mesh;
        mesh_asset asset = {};
        asset.name = mesh->name;
        asset.surfaces = (geo_surface*)malloc(sizeof(geo_surface) * mesh->primitives_count);
        if (asset.surfaces == 0)
        {
            exit(2);
        }

        vertices.clear();
        indices.clear();
 
        for (u32 j = 0; j < mesh->primitives_count; j++)
        {
            cgltf_primitive* p = &mesh->primitives[j];

            if (p->type != cgltf_primitive_type_triangles)
            {
                printf("%d\n", p->type);
            }

            if (p->indices == 0)
            {
                printf("No indices");
                exit(56);
            }

            

            geo_surface surface = {};

            cgltf_accessor* index_attrib = p->indices;


            surface.start = indices.size();
            surface.count = index_attrib->count;

            size_t init_vtx = vertices.size();


            //indices

            {
                //index_num += index_attrib->count;
                for (u32 k = 0; k < index_attrib->count; k++)
                {
                    size_t _index = cgltf_accessor_read_index(index_attrib, k);
                    indices.push_back(_index + init_vtx);
                }
            }
            

            //attributes
            //     1. Vertex
            //     2. normals
            //     3. colors
            for (u32 k = 0; k < p->attributes_count; k++)
            {
                cgltf_attribute* attrib = &p->attributes[k];

                if (attrib->type == cgltf_attribute_type_position)
                {
                    cgltf_accessor* vert_attrib = attrib->data;
                    //vertex_num += attrib->data->count;
                   // vertices.reserve(attrib->data->count);
                    for (u32 l = 0; l < attrib->data->count; l++)
                    {
                        f32 _vertices[3] = {};
                        cgltf_accessor_read_float(vert_attrib, l, _vertices, sizeof(f32));

                        if (vert_attrib->type != cgltf_type_vec3)
                        {
                            printf("q");
                            exit(69);
                        }
                        //bleh bleh bleh
                        //     -vampires
                        

                        YkVertex _v = {};
                        _v.pos.x = _vertices[0];
                        _v.pos.y = _vertices[1];
                        _v.pos.z = _vertices[2];

                        vertices.insert(vertices.begin() + l + init_vtx, _v);
                        
                        //Material colors
#if 0
                        if (p->material)
                        {

                            if (p->material->has_pbr_metallic_roughness)
                            {
                                cgltf_material* _mat = p->material;
                                f32* base_color_factor = _mat->pbr_metallic_roughness.base_color_factor;
                                f32 red = base_color_factor[0];
                                f32 green = base_color_factor[1];
                                f32 blue = base_color_factor[2];
                                f32 alpha = base_color_factor[3];

                                vertices[l + init_vtx].color = v4{ red,green,blue,alpha };
                            }
                        }
                    
#endif                       
                       

                    }
                }

                if (attrib->type == cgltf_attribute_type_normal)
                {
                    cgltf_accessor* norm_attrib = attrib->data;

                    for (u32 l = 0; l < norm_attrib->count; l++)
                    {
                        f32 _norm[3] = {};
                        cgltf_accessor_read_float(norm_attrib, l, _norm, sizeof(f32));

                        //I don't say bleh bleh bleh
                        //             -Adam Sandler
                        
                       vertices[l + init_vtx].normal.x = _norm[0];
                       vertices[l + init_vtx].normal.y = _norm[1];
                        vertices[l + init_vtx].normal.z = _norm[2];

                        if (debug_color)
                        {
                            vertices[l + init_vtx].color = v4{ _norm[0],_norm[1],_norm[2],1 };
                        }
                        
                        
                    }
                }
                if (!debug_color)
                {
                    if (attrib->type == cgltf_attribute_type_color)
                    {
                        cgltf_accessor* color_attrib = attrib->data;

                        for (u32 l = 0; l < color_attrib->count; l++)
                        {
                            f32 _color[4] = {};
                            cgltf_accessor_read_float(color_attrib, l, _color, sizeof(f32));
                            f32 red = _color[0];
                            f32 green = _color[1];
                            f32 blue = _color[2];
                            f32 alpha =_color[3];

                           // vertices[l + init_vtx].color = v4{ red,green,blue,alpha };

                        }

                    }
                }

            }

            asset.num_surfaces++;
            asset.surfaces[j] = surface;

        }
        f32 mat[16] = {};
        cgltf_node_transform_world(_node, mat);

        for (u32 i = 0; i < 4; i++)
        {
            for (u32 j = 0; j < 4; j++)
            {
                asset.model_mat[i][j] = mat[i * 4 + j];
            }
        }     

        out[mesh_index] = asset;
        out[mesh_index].buffer = ykr_upload_mesh(_renderer, vertices.data(), vertices.size(), indices.data(), indices.size());
        mesh_index++;
        

    }
    


    for (u32 _node_index = 0; _node_index < _node->children_count; _node_index++)
    {
        cgltf_node* __node = _node->children[_node_index];
        traverse_node(__node);
    }


}

mesh_asset* yk_load_mesh(YkRenderer* renderer, const char* filepath, void* memory, size_t size, size_t* out_num_meshes)
{

    out = 0;

    YkMemoryArena index_arena;
    yk_memory_arena_innit(&index_arena, size / 2, memory);

    YkMemoryArena vertex_arena;
    yk_memory_arena_innit(&vertex_arena, size / 2, (u8*)memory + size / 2);

    vertices.resize(1000000);
    indices.resize(10000000);

    cgltf_options options = {};
    cgltf_data* data = NULL;

    indices = {};
    vertices = {};
    //indices = (u32*)index_arena.base;
    //vertices = (YkVertex*)vertex_arena.base;

    _renderer = renderer;
   
    if (cgltf_parse_file(&options, filepath, &data) == cgltf_result_success)
    {

        if (cgltf_load_buffers(&options, data, filepath) != cgltf_result_success)
        {
            printf("Couldn't load buffers");
        }



        out = (mesh_asset*)malloc(sizeof(mesh_asset) * data->meshes_count);
        *out_num_meshes = data->meshes_count;


        for (u32 _scene_index = 0; _scene_index < data->scenes_count; _scene_index++)
        {
            
            cgltf_scene* _scene = &data->scenes[_scene_index];

            for (u32 _node_index = 0; _node_index < _scene->nodes_count; _node_index++)
            {
                cgltf_node* _node = _scene->nodes[_node_index];

                traverse_node(_node);
            }
              

        }

    }

    return out;
}
     
      
       /*
        for (u32 i = 0, mesh_index = 0; i < data->meshes_count; i++) 
        {
            cgltf_mesh mesh = data->meshes[i];

            mesh_asset new_mesh = {};
            new_mesh.name = mesh.name;
            
            index_num = 0;
            vertex_num = 0;
            
            new_mesh.num_surfaces = 0;
            new_mesh.surfaces = (geo_surface*)malloc(sizeof(geo_surface) * data->meshes[i].primitives_count);

            for (u32 j = 0; j < data->meshes[i].primitives_count; j++) 
            {
                cgltf_primitive* p = &mesh.primitives[j];

                if (p->type != cgltf_primitive_type_triangles) continue;

                cgltf_accessor* index_attrib = data->meshes[i].primitives[j].indices;
                
                geo_surface surface;
                surface.start = index_num;
                surface.count = index_attrib->count;

                size_t initial_vtx = vertex_num;
                
                //load indices  
                if (index_attrib->component_type == cgltf_component_type_r_16u) 
                {
                    cgltf_size index;
                    for (size_t k = 0; k < index_attrib->count; ++k) 
                    {
                        index = cgltf_accessor_read_index(index_attrib, k);
                        indices[k] = index + initial_vtx;
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
                            for (size_t l = 0; l < attribute->count; ++l) 
                            {
                                float* values = (float*)attribute->buffer_view->buffer->data + attribute->offset / sizeof(float) + l * 3;
                                vertices[l + initial_vtx].pos.x = values[0];
                                vertices[l + initial_vtx].pos.y = values[1];
                                vertices[l + initial_vtx].pos.z = values[2];
                                

                               // yk_memory_arena_push(&vertex_arena, sizeof(YkVertex), values);
                                vertex_num++;
                              
                            }
                        }
                    }
                                                      
                    
                    if(p->attributes[k].type == cgltf_attribute_type_normal)    //NORMAL
                    {
                        cgltf_accessor* attribute = p->attributes[k].data;
                        
                        for (size_t l = 0; l < attribute->count; ++l)
                        {
                            float* values = (float*)attribute->buffer_view->buffer->data + attribute->offset / sizeof(float) + l * 3;
                            vertices[l + initial_vtx].normal.x = values[0];
                            vertices[l + initial_vtx].normal.y = values[1];
                            vertices[l + initial_vtx].normal.z = values[2];
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
            out[i] = new_mesh;
            printf("Mesh %u: Name: %s, Num Surfaces: %zu\n", i, new_mesh.name, new_mesh.num_surfaces);

        }
       
       */

/*
    }
    else
    {
        printf("couldn't parse file");
    }
    printf("out data\n");

    for (u32 i = 0; i < *out_num_meshes; i++)
    {
        mesh_asset* mesh = &out[i];
        printf("Mesh %u: Name: %s, Num Surfaces: %zu\n", i, mesh->name ,mesh->num_surfaces);
    }

    

    for (size_t i = 0; i < index_num; i++)
    {
        printf("index %zu: %zu\n", i, indices[i]);
    }
   
    for (size_t i = 0; i < vertex_num; i++)
    {
        ykm_print_v3(vertices[i].pos);
    }
*/

/*

            if (index_attrib->component_type == cgltf_component_type_r_32u)
            {
                index_num += index_attrib->count;
                for (u32 k = 0; k < index_attrib->count; k++)
                {
                    size_t _index = cgltf_accessor_read_index(index_attrib, k);
                    indices[k] = _index + init_vtx;
                }
            }
*/