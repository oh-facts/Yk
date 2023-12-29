#ifndef YK_RENDERER_MODEL_H
#define YK_RENDERER_MODEL_H

#include "common.h"
#include <yk_math.h>

typedef struct ubo ubo;
typedef struct vertex vertex;
typedef struct buffer buffer;
typedef struct model model;

struct ubo
{
    m4 model;
    m4 view;
    m4 proj;
};

struct vertex
{
    v2 pos;
    v3 color;
};


struct buffer
{
    VkBuffer handle;
    VkDeviceMemory memory;
};


struct model
{
    vertex* vertices;
    u32* indices;
   
    struct buffer vert_buffer;
    struct buffer index_buffer;
    struct ubo ubo;
};

#include "renderer.h"


void yk_model_init(struct yk_renderer* ren, vertex vertices[], u16 indices[], model* modela);

void createUniformBuffers(struct yk_renderer* renderer);
void updateUniformBuffer(struct yk_renderer* renderer, uint32_t currentImage);

#endif // !YK_RENDERER_MODEL_H
