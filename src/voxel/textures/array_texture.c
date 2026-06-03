#include "array_texture.h"

#include <stddef.h>

#include "base_texture_enum.h"

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

GLuint load_block_texture_array(void)
{
    ArrayAtlas atlas;
    atlas_init(&atlas);
    for (BaseTextureEnum i = 0; i < TEXTURE_COUNT; i++)
        atlas_push_from_base_texture(&atlas, i, 1);
    GLuint tex = atlas_upload_to_gpu(&atlas);
    atlas_free(&atlas);
    return tex;
}
