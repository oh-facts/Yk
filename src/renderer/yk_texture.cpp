#include <renderer/yk_texture.h>
#include <stb/stb_image.h>

YkImageData yk_image_load_data(const char* path)
{
    YkImageData out = { };
    stbi_set_flip_vertically_on_load(1);
    out.data = stbi_load(path,(i32*)& out.width, (i32*)&out.height, (i32*)&out.num_channel, STBI_rgb_alpha);
 

    Assert(out.data, "Failed to read image")
    return out;
}


void yk_image_data_free(YkImageData* self)
{
    stbi_image_free(self->data);
}