#ifndef YK_TEXTURE_H
#define YK_TEXTURE_H

#include <renderer/mn_types.h>

struct YkTexture
{
	u8* data;
	i32 width;
	i32 height;
	i32 num_channel;
};

YkTexture yk_texture_create(const char* path);
void yk_texture_free(YkTexture* texture);
#endif