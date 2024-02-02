#ifndef YK_TEXTURE_H
#define YK_TEXTURE_H

#include <renderer/ykr_common.h>

struct YkImageData
{
	u8* data;
	u32 width;
	u32 height;
	u32 num_channel;
};

YkImageData yk_image_load_data(const char* path);
void yk_image_data_free(YkImageData* self);
#endif