#pragma once

#include <stdint.h>

#include "voxel/textures/face_texture.h"

typedef enum BlockType : uint16_t
{
    BLK_AIR = 0,
    BLK_GRASS,
    BLK_DIRT,
    BLK_STONE,
    BLK_DARKSTONE,
    BLK_OAK,
    BLK_FOLIAGE,
    BLK_SAND,
    BLK_RED_SAND,
    BLK_GRAVEL,
    BLK_SNOW,
    BLOCK_COUNT
} BlockType;

#define FACE_COUNT 6

typedef struct Block
{
    BlockType type;
} Block;

extern const FaceTextureBuilder BlockFaceBuilders[][FACE_COUNT];
extern const int BlockFaceBuildersCount;

extern FaceTexture BlockFaces[][FACE_COUNT];
