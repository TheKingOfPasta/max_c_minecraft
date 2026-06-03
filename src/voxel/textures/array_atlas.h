#pragma once

#include <stdint.h>

#include "base_texture_enum.h"

#define TEXTURE_WIDTH_HEIGHT 16
#define MAX_TEXTURE 600

typedef struct
{
    uint32_t count;
    uint8_t** pixels; // pixels[MAX_TEXTURE][TEXTURE_WIDTH_HEIGHT * 4]
} ArrayAtlas;

void atlas_init(ArrayAtlas* a);
void atlas_free(ArrayAtlas* a);

uint16_t atlas_get_current_slot(ArrayAtlas* a);
void atlas_push_from_base_texture(ArrayAtlas* a, BaseTextureEnum bid,
                                  uint16_t frame_count);

uint8_t* load_rgba(const char* path);
