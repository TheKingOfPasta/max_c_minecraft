#include "array_texture.h"

#include <stddef.h>

#include "array_atlas.h"
#include "voxel/block.h"

static GLuint atlas_upload_to_gpu(ArrayAtlas* a)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT,
                 a->count, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (uint32_t i = 0; i < a->count; i++)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)i, TEXTURE_WIDTH_HEIGHT,
                        TEXTURE_WIDTH_HEIGHT, 1, GL_RGBA, GL_UNSIGNED_BYTE, a->pixels[i]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return tex;
}

static void build_texture_look_up(const FaceTextureBuilder builders[][FACE_COUNT],
                                  FaceTexture destination[][FACE_COUNT], ArrayAtlas* atlas)
{
    for (int b = 0; b < BlockFaceBuildersCount; b++)
        for (int f = 0; f < FACE_COUNT; f++)
            destination[b][f] = face_texture_build(&builders[b][f], atlas);
}

GLuint load_block_texture_array(void)
{
    ArrayAtlas atlas;
    atlas_init(&atlas);
    build_texture_look_up(BlockFaceBuilders, BlockFaces, &atlas);
    GLuint tex = atlas_upload_to_gpu(&atlas);
    atlas_free(&atlas);
    return tex;
}

void texture_array_init(void)
{
    GLuint tex_array = load_block_texture_array();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);
}
