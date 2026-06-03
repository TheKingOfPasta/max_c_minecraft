#include "face_texture.h"

#include <assert.h>

#include "voxel/textures/array_atlas.h"

uint16_t face_texture_resolve(const FaceTexture* f, [[maybe_unused]] uint8_t neighbor_mask,
                              bool activated, uint8_t hash_pos)
{
    const uint8_t connected_size = (f->flags & FACETXT_CONNECTED8_BIT) ? 46
        : (f->flags & FACETXT_CONNECTED4_BIT)                          ? 16
                                                                       : 1;
    const uint8_t activated_size = (f->flags & FACETXT_ACTIVATED_BIT) ? 2 : 1;

    const uint8_t variant_pos = (f->flags & FACETXT_VARIANT_BIT) ? hash_pos % f->variant_count : 0;
    const uint8_t activated_pos = (f->flags & FACETXT_ACTIVATED_BIT) ? (activated ? 1 : 0) : 0;
    const uint8_t connected_pos = 0;
    // (connected_size > 1) ? connected_mask_to_offset[neighbor_mask] :
    // 0;

    int res = f->base_id
        + ((uint32_t)variant_pos * activated_size * connected_size
           + (uint32_t)activated_pos * connected_size + (uint32_t)connected_pos)
            * f->frame_count;
    //    printf("resolved :%d\n", res);
    return res;
}

static void consume_leaf(const FaceTextureBuilder* fb, FaceTexture* res, ArrayAtlas* atlas)
{
    if (res->frame_count == 0)
        res->frame_count = fb->leaf.count;
    assert(res->frame_count == fb->leaf.count);

    atlas_push_from_base_texture(atlas, fb->leaf.texture_id, fb->leaf.count);
}

static void consume_connected(const FaceTextureBuilder* fb, FaceTexture* res, ArrayAtlas* atlas)
{
    if (!(fb->type & (FACETXT_CONNECTED4_BIT | FACETXT_CONNECTED8_BIT)))
    {
        consume_leaf(fb, res, atlas);
        return;
    }

    res->flags |=
        (fb->type & FACETXT_CONNECTED4_BIT) ? FACETXT_CONNECTED4_BIT : FACETXT_CONNECTED8_BIT;

    uint8_t cases = (fb->type & FACETXT_CONNECTED8_BIT) ? 46 : 16;

    for (uint8_t c = 0; c < cases; c++)
    {
        const FaceTextureBuilder* src;
        switch (0) // connected_offset_to_part[c])
        {
        case 0:
            src = fb->connected.inner;
            break;
        case 1:
            src = fb->connected.edge;
            break;
        default:
            src = fb->connected.corner;
            break;
        }
        consume_leaf(src, res, atlas);
    }
}

static void consume_activated(const FaceTextureBuilder* fb, FaceTexture* res, ArrayAtlas* atlas)
{
    if (!(fb->type & FACETXT_ACTIVATED_BIT))
    {
        consume_connected(fb, res, atlas);
        return;
    }
    res->flags |= FACETXT_ACTIVATED_BIT;
    consume_connected(fb->activated.off, res, atlas);
    consume_connected(fb->activated.on, res, atlas);
}

static void consume_variant(const FaceTextureBuilder* fb, FaceTexture* res, ArrayAtlas* atlas)
{
    if (!(fb->type & FACETXT_VARIANT_BIT))
    {
        consume_activated(fb, res, atlas);
        return;
    }
    res->flags |= FACETXT_VARIANT_BIT;
    res->variant_count = fb->variant.count;
    for (uint8_t i = 0; i < fb->variant.count; i++)
        consume_activated(fb->variant.children[i], res, atlas);
}

FaceTexture face_texture_build(const FaceTextureBuilder* fb, ArrayAtlas* atlas)
{
    FaceTexture res = {
        .base_id = atlas_get_current_slot(atlas),
        .flags = 0,
        .variant_count = 1,
        .frame_count = 0,
    };

    consume_variant(fb, &res, atlas);

    return res;
}
