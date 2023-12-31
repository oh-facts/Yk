#include <renderer/yk_texture.h>
#include <stb/stb_image.h>

YkTexture yk_texture_create(const char* path)
{
    YkTexture out = { };
    out.data = stbi_load(path,&out.width,&out.height,&out.num_channel,STBI_rgb_alpha);

    Assert(out.data, "Failed to read image")
    return out;
}

void yk_texture_free(YkTexture* texture)
{
    stbi_image_free(texture->data);
}