#include <renderer/ykr_model_loader.h>
#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include <cgltf/cgltf_write.h>
#include <renderer/renderer.h>
#include <renderer/yk_texture.h>

// ===================internal=======================
#define ROOT_PATH_SIZE 256
yk_internal char m_root_path[ROOT_PATH_SIZE];

/*
    So, gltf returns a texture path like textures/shinchan_1.jpg. But
    I need absolute paths. So I append res/models/shinchan/. That  is
    the purpose of this function.
*/
yk_internal void join_paths(const char *model_path, const char *texture_path, char *joined_path);

/*
    Clean up global state. Unrequired since variables
    don't  presist between  function calls. Might add
    an #ifdef to not do it at all. It exists  because
    some  values  used to  persist and hot  reloading
    killed  them. So  I  got bugs. This  fn exists to
    remind us of that story.
*/
yk_internal void ykr_load_mesh_cleanup();
// =================================================

// dear lawd, pwease forgive me
// =============non-persistent-globals=================

/*
    These variables don't persist. There is no global
    state.  They  are static  to  make life easy (see
    recursive  function below). If you can find a way
    to  avoid  them without  making  things messy. Go
    ahead.  Also, this  is  a "start-up" fn that runs
    only when loading new  models for the  first time.
    So  I am not too concerned. However, feel free to
    suggest. If you're suggestion is a capture lambda,
    I will (taylor) swiftly ignore you.
*/

yk_internal load_mesh_scratch_arena *m_scratch;

yk_internal YkRenderer *m_renderer;

yk_internal size_t m_total_indices;
yk_internal size_t m_total_vertices;

yk_internal model_assets *m_model;

// =================================================

#define debug_color 0
#define material_color 1

// for fun
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

yk_internal void ykr_load_mesh_cleanup()
{
    m_renderer = 0;
    m_total_vertices = 0;
    m_total_indices = 0;
    m_model = 0;
    m_scratch = 0;
    memset(m_root_path, 0, ROOT_PATH_SIZE);
}

void traverse_node(cgltf_node *_node)
{

    if (_node->mesh)
    {

        cgltf_mesh *_mesh = _node->mesh;

        mesh_asset asset = {};
        asset.name = _mesh->name;
        asset.surface_count = _mesh->primitives_count;

        m_model->surface_count += _mesh->primitives_count;

        YkVertex *m_vertices = (YkVertex *)m_scratch->vertices.base;
        u32 *m_indices = (u32 *)m_scratch->indices.base;
        size_t m_vertex_num = 0;
        size_t m_index_num = 0;

        for (u32 i = 0; i < _mesh->primitives_count; i++)
        {
            cgltf_primitive *p = &_mesh->primitives[i];

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

            cgltf_accessor *index_attrib = p->indices;

            surface.start = m_index_num;

            surface.count = index_attrib->count;

            size_t init_vtx = m_vertex_num;

            // indices

            {
                m_scratch->indices.used += index_attrib->count * sizeof(size_t);
                Assert(m_scratch->indices.size >= m_scratch->indices.used + sizeof(size_t) * index_attrib->count, "WOAH. Thats a lot of indices");
                for (u32 j = 0; j < index_attrib->count; j++)
                {
                    size_t _index = cgltf_accessor_read_index(index_attrib, j);

                    m_indices[j + m_index_num] = _index + init_vtx;

                    // here
                }

                m_index_num += index_attrib->count;
            }

            // attributes
            //      1. Vertex
            //      2. normals
            //      3. colors
            //      4. uv
            {
                for (u32 j = 0; j < p->attributes_count; j++)
                {
                    cgltf_attribute *attrib = &p->attributes[j];

                    if (attrib->type == cgltf_attribute_type_position)
                    {
                        cgltf_accessor *vert_attrib = attrib->data;
                        m_vertex_num += attrib->data->count;

                        m_scratch->vertices.used += attrib->data->count * sizeof(YkVertex);
                        Assert(m_scratch->vertices.size >= m_scratch->vertices.used + sizeof(YkVertex) * attrib->data->count, "WOAH. Thats a lot of vertices");

                        for (u32 k = 0; k < attrib->data->count; k++)
                        {
                            f32 _vertices[3] = {};
                            cgltf_accessor_read_float(vert_attrib, k, _vertices, sizeof(f32));

                            if (vert_attrib->type != cgltf_type_vec3)
                            {
                                printf("q");
                                exit(69);
                            }
                            // bleh bleh bleh
                            //      -vampires
                            m_vertices[k + init_vtx].pos.x = _vertices[0];
                            m_vertices[k + init_vtx].pos.y = _vertices[1];
                            m_vertices[k + init_vtx].pos.z = _vertices[2];
                            // Material colors
#if material_color
                            if (p->material)
                            {
                                if (p->material->has_pbr_metallic_roughness)
                                {
                                    cgltf_material *_mat = p->material;
                                    f32 *base_color_factor = _mat->pbr_metallic_roughness.base_color_factor;
                                    f32 red = base_color_factor[0];
                                    f32 green = base_color_factor[1];
                                    f32 blue = base_color_factor[2];
                                    f32 alpha = base_color_factor[3];

                                    surface.material.base_color = v4{red, green, blue, alpha};

                                }
                            }
#endif
                        }
                    }

                    if (attrib->type == cgltf_attribute_type_normal)
                    {
                        cgltf_accessor *norm_attrib = attrib->data;

                        for (u32 k = 0; k < norm_attrib->count; k++)
                        {
                            f32 _norm[3] = {};
                            cgltf_accessor_read_float(norm_attrib, k, _norm, sizeof(f32));

                            // I don't say bleh bleh bleh
                            //              -Adam Sandler

                            m_vertices[k + init_vtx].normal.x = _norm[0];
                            m_vertices[k + init_vtx].normal.y = _norm[1];
                            m_vertices[k + init_vtx].normal.z = _norm[2];

#if debug_color
                            m_vertices[l + init_vtx].color = v4{_norm[0], _norm[1], _norm[2], 1};
#endif
                        }
                    }

                    if (attrib->type == cgltf_attribute_type_color)
                    {
                        cgltf_accessor *color_attrib = attrib->data;

                        for (u32 k = 0; k < color_attrib->count; k++)
                        {
                            f32 _color[4] = {};
                            cgltf_accessor_read_float(color_attrib, k, _color, sizeof(f32));
                            f32 red = _color[0];
                            f32 green = _color[1];
                            f32 blue = _color[2];
                            f32 alpha = _color[3];

                            m_vertices[k + init_vtx].color = v4{red, green, blue, alpha};
                        }
                    }

                    if (attrib->type == cgltf_attribute_type_texcoord)
                    {
                        cgltf_accessor *uv_attrib = attrib->data;
                        if (attrib->index == 0)
                        {
                            for (u32 k = 0; k < uv_attrib->count; k++)
                            {
                                f32 _uv[2] = {};
                                cgltf_accessor_read_float(uv_attrib, k, _uv, sizeof(f32));

                                m_vertices[k + init_vtx].uv_x = _uv[0];
                                m_vertices[k + init_vtx].uv_y = _uv[1];
                            }
                        }
                    }
                }
            }

            cgltf_material *material = p->material;

            if (material->has_pbr_metallic_roughness)
            {
                cgltf_texture_view *base_view = &material->pbr_metallic_roughness.base_color_texture;
                if (base_view->texture)
                {
                    size_t texture_count = arena_count(m_renderer->textures, texture_asset);
                    char fullpath[ROOT_PATH_SIZE] = {};
                    join_paths(m_root_path, base_view->texture->image->uri, fullpath);

                    u64 hash = djb2_hash(fullpath);

                    texture_asset *ass = (texture_asset *)m_renderer->textures.base;

                    // ToDo(facts): Instead of assinging hash, assign array index
                    // so I can use descr indexing.
                    for (u32 j = 0; j < texture_count; j++)
                    {
                        if (ass[j].id == hash)
                        {
                            surface.material.texture_id = hash;
                            // printf("%llu\n",hash);
                            break;
                        }
                    }
                }
            }

            arena_push(m_model->surfaces, geo_surface, surface);
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

        asset.buffer = ykr_upload_mesh(m_renderer, m_vertices, m_vertex_num, m_indices, m_index_num);

        arena_push(m_model->meshes, mesh_asset, asset);

        m_total_indices += m_index_num;
        m_total_vertices += m_vertex_num;

        // yk_memory_arena_clean_reset(&m_scratch->indices);
        // yk_memory_arena_clean_reset(&m_scratch->vertices);
    }

    for (u32 _node_index = 0; _node_index < _node->children_count; _node_index++)
    {
        cgltf_node *__node = _node->children[_node_index];
        traverse_node(__node);
    }
}

void ykr_load_mesh(YkRenderer *renderer, const char *filepath, load_mesh_scratch_arena *scratch, model_assets *inmodel)
{
    ykr_load_mesh_cleanup();

    // feel free to suggest better method
    m_model = inmodel;
    strcpy(m_root_path, filepath);

    cgltf_options options = {};
    cgltf_data *data = 0;

    m_renderer = renderer;

    m_scratch = scratch;

    if (cgltf_parse_file(&options, filepath, &data) == cgltf_result_success)
    {

        if (cgltf_load_buffers(&options, data, filepath) != cgltf_result_success)
        {
            printf("Couldn't load buffers");
            exit(99);
        }

        m_model->mesh_count += data->meshes_count;

        arena_push(m_model->per_model, size_t, data->meshes_count);

        // load textures
        for (u32 i = 0; i < data->textures_count; i++)
        {
            char fullpath[ROOT_PATH_SIZE] = {};
            join_paths(m_root_path, data->textures[i].image->uri, fullpath);
            printf("%s\n", fullpath);

            texture_asset texture = ykr_load_textures(renderer, fullpath);
            texture.id = djb2_hash(fullpath);
            texture.name = data->textures[i].image->name;
            arena_push(renderer->textures, texture_asset, texture);
        }

        /*
                for(u32 i = 0; i < data->textures_count; i ++)
                {

                    printf("%llu\n", view[i].id);

                }
        */

        printf("%s\n", filepath);
        for (u32 _scene_index = 0; _scene_index < data->scenes_count; _scene_index++)
        {

            cgltf_scene *_scene = &data->scenes[_scene_index];

            for (u32 _node_index = 0; _node_index < _scene->nodes_count; _node_index++)
            {
                cgltf_node *_node = _scene->nodes[_node_index];

                traverse_node(_node);
            }
        }
    }

    cgltf_free(data);
}

yk_internal void join_paths(const char *model_path, const char *texture_path, char *joined_path)
{

    const char *lastSlash = strrchr(model_path, '/');

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