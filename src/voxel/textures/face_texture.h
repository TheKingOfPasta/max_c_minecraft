#pragma once

#include <stdint.h>

#include "voxel/textures/array_atlas.h"

typedef struct
{
    uint16_t base_id;
    uint8_t flags;
    uint8_t variant_count;
    uint8_t frame_count;
} FaceTexture;

typedef enum : uint8_t
{
    FACETXT_CONNECTED4_BIT = 1 << 0,
    FACETXT_CONNECTED8_BIT = 1 << 1,
    FACETXT_ACTIVATED_BIT = 1 << 2,
    FACETXT_VARIANT_BIT = 1 << 3,
} FaceTextureFlags;

typedef struct FaceTextureBuilder
{
    FaceTextureFlags type;
    union
    {
        struct
        {
            uint32_t texture_id;
            uint8_t count;
        } leaf;
        struct
        {
            struct FaceTextureBuilder* inner;
            struct FaceTextureBuilder* edge;
            struct FaceTextureBuilder* corner;
        } connected;
        struct
        {
            struct FaceTextureBuilder* off;
            struct FaceTextureBuilder* on;
        } activated;
        struct
        {
            struct FaceTextureBuilder** children;
            uint8_t count;
        } variant;
    };
} FaceTextureBuilder;

uint16_t face_texture_resolve(const FaceTexture* f, uint8_t neighbor_mask,
                              bool activated, uint8_t hash_pos);

FaceTexture face_texture_build(const FaceTextureBuilder* fb, ArrayAtlas* atlas);
