#include "block.h"

#include "textures/base_texture_enum.h"
#include "textures/face_texture.h"

#define BARREL(SIDE, TOPBOT) { (SIDE), (SIDE), (SIDE), (SIDE), (TOPBOT), (TOPBOT) }

#define DONUT(SIDE, TOP, BOT) { (SIDE), (SIDE), (SIDE), (SIDE), (TOP), (BOT) }

#define UNIFO(TEXT) { (TEXT), (TEXT), (TEXT), (TEXT), (TEXT), (TEXT) }

#define CROWN(SIDE, FRONT, BOT, TOP) { (SIDE), (FRONT), (TOP), (BOT), (SIDE), (SIDE) }

#define LEAF(TID)                                                                                  \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = 0, .leaf = { .texture_id = (TID), .count = 1 },                                    \
    }
#define LEAF_ANIM(TID, N)                                                                          \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = 0, .leaf = { .texture_id = (TID), .count = (N) },                                  \
    }
#define CONNECTED4(INNER, EDGE, CORN)                                                              \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = FACETXT_CONNECTED4_BIT,                                                            \
        .connected = { .inner = &(INNER), .edge = &(EDGE), .corner = &(CORN) },                    \
    }
#define CONNECTED8(INNER, EDGE, CORN)                                                              \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = FACETXT_CONNECTED8_BIT,                                                            \
        .connected = { .inner = &(INNER), .edge = &(EDGE), .corner = &(CORN) },                    \
    }
#define ACTIVATED(OFF, ON)                                                                         \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = FACETXT_ACTIVATED_BIT, .activated = { .off = &(OFF), .on = &(ON) },                \
    }
#define VARIANT(...)                                                                               \
    (FaceTextureBuilder)                                                                           \
    {                                                                                              \
        .type = FACETXT_VARIANT_BIT,                                                               \
        .variant = {                                                                               \
            .children = (FaceTextureBuilder*[]){ __VA_ARGS__ },                                    \
            .count = sizeof((FaceTextureBuilder[]){ __VA_ARGS__ }) / sizeof(FaceTextureBuilder),   \
        },                                                                                         \
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
const FaceTextureBuilder BlockFaceBuilders[][FACE_COUNT] = {
    [BLK_DIRT] = UNIFO(LEAF(TEX_DIRT)),
    [BLK_GRASS] = DONUT(LEAF(TEX_GRASS_SIDE), LEAF(TEX_GRASS), LEAF(TEX_DIRT)),
    [BLK_STONE] = UNIFO(LEAF(TEX_STONE)),
    [BLK_OAK] = BARREL(LEAF(TEX_OAK), LEAF(TEX_OAK_INNER)),
};
#pragma GCC diagnostic pop

const int BlockFaceBuildersCount = (sizeof(BlockFaceBuilders) / sizeof(*BlockFaceBuilders));

#undef BARREL
#undef DONUT
#undef UNIFO
#undef CROWN

#undef LEAF
#undef VARIANT
#undef ACTIVATED
#undef LEAF_ANIM
#undef CONNECTED4
#undef CONNECTED8

FaceTexture BlockFaces[BLOCK_COUNT][FACE_COUNT];
