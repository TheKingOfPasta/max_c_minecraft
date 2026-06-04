#include "array_atlas.h"

#include "base_texture_enum.h"
#include "utils/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void atlas_init(ArrayAtlas* a)
{
    a->count = 0;
    a->pixels = malloc(sizeof(uint8_t*) * MAX_TEXTURE);
}

uint16_t atlas_get_current_slot(ArrayAtlas* a)
{
    return a->count;
}

uint8_t* load_rgba(const char* path)
{
    int iw, ih, ch;
    uint8_t* px = stbi_load(path, &iw, &ih, &ch, 4);

    ASSERT(px, "failed to load texture %s", path);
    ASSERT(iw == TEXTURE_WIDTH_HEIGHT && ih == TEXTURE_WIDTH_HEIGHT,
           "all textures must be same size");

    return px;
}

static uint32_t atlas_push(ArrayAtlas* a, uint8_t* rgba)
{
    ASSERT(a->count < MAX_TEXTURE, "reached max cap in atlas");
    a->pixels[a->count++] = rgba;
    return a->count - 1;
}

void atlas_push_from_base_texture(ArrayAtlas* a, BaseTextureEnum bid,
                                  uint16_t frame_count)
{
    for (int i = 0; i < frame_count; i++)
        atlas_push(a, load_rgba(TexturePaths[bid + i]));
}

void atlas_free(ArrayAtlas* a)
{
    for (uint32_t i = 0; i < a->count; i++)
        stbi_image_free(a->pixels[i]);
    free(a->pixels);
}
