#include <renderer/ykr_model_loader.h>
#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include <cgltf/cgltf_write.h>
#include <renderer/renderer.h>
#include <renderer/yk_texture.h>

yk_internal texture_asset ykr_load_textures(const YkRenderer* renderer, const char* filepath);
yk_internal void join_paths(const char* model_path, const char* texture_path, char* joined_path);

//dear lawd, pwease forgive me
yk_internal u32 mesh_index;

yk_internal YkVertex* vertices;
yk_internal u32* indices;
yk_internal geo_surface* surfaces;

yk_internal size_t index_num;
yk_internal size_t vertex_num;

yk_internal mesh_asset* out;
yk_internal const YkRenderer* _renderer;
#define ROOT_PATH_SIZE 256
yk_internal char root_path[ROOT_PATH_SIZE];

#define debug_color 0
#define material_color 1

//perf reasons
/*
* some values
*
fire_in_the_sky
v: 2821300
i: 2839425

fire_in_the_sky2 (like previous, but all vertices joined)
v: 2821300
i: 2825133
m:

sponza
v: 192496
i: 786801
m: 1
s: 103
*/

yk_internal size_t total_vertices;
yk_internal size_t total_indices;
yk_internal size_t total_meshes;
yk_internal size_t total_surfaces;

void ykr_load_mesh_cleanup()
{
    mesh_index = 0;
    vertices = 0;
    indices = 0;
    surfaces = 0;
    index_num = 0;
    vertex_num = 0;
    out = 0;
    _renderer = 0;
    total_vertices = 0;
    total_indices = 0;
    memset(root_path, 0,ROOT_PATH_SIZE);
    //total_meshes = 0;
   // total_surfaces = 0;
}

void traverse_node(cgltf_node* _node)
{

    if (_node->mesh)
    {

        cgltf_mesh* mesh = _node->mesh;
        mesh_asset asset = {};
        asset.name = mesh->name;
        asset.surfaces = (geo_surface*)((u8*)surfaces + sizeof(geo_surface) * total_surfaces);

        total_surfaces += mesh->primitives_count;
        if (asset.surfaces == 0)
        {
            exit(2);
        }

        index_num = 0;
        vertex_num = 0;

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

            surface.start = index_num;;
            surface.count = index_attrib->count;

            size_t init_vtx = vertex_num;


            //indices

            {

                for (u32 k = 0; k < index_attrib->count; k++)
                {
                    size_t _index = cgltf_accessor_read_index(index_attrib, k);

                    indices[k + index_num] = _index + init_vtx;

                    //here
                }

                index_num += index_attrib->count;
            }

            //attributes
            //     1. Vertex
            //     2. normals
            //     3. colors
            //     4. uv
            {
                for (u32 k = 0; k < p->attributes_count; k++)
                {
                    cgltf_attribute* attrib = &p->attributes[k];


                    if (attrib->type == cgltf_attribute_type_position)
                    {
                        cgltf_accessor* vert_attrib = attrib->data;
                        vertex_num += attrib->data->count;
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

                            vertices[l + init_vtx].pos.x = _vertices[0];
                            vertices[l + init_vtx].pos.y = _vertices[1];
                            vertices[l + init_vtx].pos.z = _vertices[2];

                            //yk_memory_arena_insert(&vertex_arena, sizeof(YkVertex), l + init_vtx, &_v);


                            //Material colors
#if material_color
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
                                    vertices[l + init_vtx].color = v4{ red, green, blue, alpha };
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

#if debug_color
                            vertices[l + init_vtx].color = v4{ _norm[0], _norm[1], _norm[2], 1 };
#endif                     


                        }
                    }

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
                            f32 alpha = _color[3];

                            vertices[l + init_vtx].color = v4{ red, green, blue, alpha };
                        }

                    }

                    if (attrib->type == cgltf_attribute_type_texcoord)
                    {
                        cgltf_accessor* uv_attrib = attrib->data;
                        if (attrib->index == 0)
                        {
                            for (u32 l = 0; l < uv_attrib->count; l++)
                            {
                                f32 _uv[2] = {};
                                cgltf_accessor_read_float(uv_attrib, l, _uv, sizeof(f32));

                                vertices[l + init_vtx].uv_x = _uv[0];
                                vertices[l + init_vtx].uv_y = _uv[1];
                            }
                        }

                    }


                }
            }
            

            cgltf_material* material = p->material;

            if (material->has_pbr_metallic_roughness)
            {
                cgltf_texture_view* base_view = &material->pbr_metallic_roughness.base_color_texture;
                if (base_view->texture)
                {
                    /*
                        sowwy, I really don't know how else to handle this
                    */
                    char fullpath[ROOT_PATH_SIZE] = {};
                    join_paths(root_path, base_view->texture->image->uri,fullpath);
                    
                    asset.image = ykr_load_textures(_renderer, fullpath);
                    //printf("%s\n", asset.base_texture_path);
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
        out[mesh_index].buffer = ykr_upload_mesh(_renderer, vertices, vertex_num, indices, index_num);
        mesh_index++;

        total_indices += index_num;
        total_vertices += vertex_num;
        total_meshes++;

    }



    for (u32 _node_index = 0; _node_index < _node->children_count; _node_index++)
    {
        cgltf_node* __node = _node->children[_node_index];
        traverse_node(__node);
    }


}

mesh_asset* ykr_load_mesh(const YkRenderer* renderer, const char* filepath, YkMemoryArena* scratch, YkMemoryArena* perm, size_t * num_mesh)
{
    //feel free to suggest better method
    ykr_load_mesh_cleanup();
    strcpy(root_path, filepath);

    out = 0;
    cgltf_options options = {};
    cgltf_data* data = 0;

    _renderer = renderer;

    if (cgltf_parse_file(&options, filepath, &data) == cgltf_result_success)
    {

        if (cgltf_load_buffers(&options, data, filepath) != cgltf_result_success)
        {
            printf("Couldn't load buffers");
        }
     
        
        indices = (u32*)(scratch->base);
        vertices = (YkVertex*)((u8*)indices + scratch->size / 2);
        

        // Maybe have two arenas? So I can actually make use of the "used" part?
        // Right now I partition this arena in part_ratio : 1
        

        u32 part_ratio = 4;
        out = (mesh_asset*)(perm->base + total_meshes * sizeof(mesh_asset));
        surfaces = (geo_surface*)((u8*)out + ( perm->size - perm->size / part_ratio) + total_surfaces * sizeof(geo_surface));
        

        *num_mesh = data->meshes_count;


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

    //a bit pointless since I am using the same arena.
    //Maybe I can have an array of used and available pairs?
    perm->used += total_meshes * sizeof(mesh_asset);
    perm->used += total_surfaces * sizeof(surfaces);

    return out;
}

yk_internal texture_asset ykr_load_textures(const YkRenderer* renderer, const char* filepath)
{
    texture_asset out = {};

    YkImageData rawData = yk_image_load_data(filepath);
     out.image = ykr_create_image_from_data(renderer, rawData.data,
        VkExtent3D{ .width = rawData.width, .height = rawData.height, .depth = 1 },
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT);
    yk_image_data_free(&rawData);

    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    //set anistoropyptosy

    info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(renderer->device, &info, 0, &out.sampler);

    return out;
}

yk_internal void join_paths(const char* model_path, const char* texture_path, char* joined_path) 
{

    const char* lastSlash = strrchr(model_path, '/');

    if (lastSlash != NULL) 
    {
        size_t model_len = lastSlash - model_path + 1;

        strncpy(joined_path, model_path, model_len);

        strcat(joined_path, texture_path);
    } 
    else 
    {

        printf("crash");
    }
}