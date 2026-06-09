#include "mesh_gen.h"

#include "opengl/tracy.h"
#include "utils/container.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

static inline VEC3(i32) get_dir_vector(int x, int y, int dir, int val, int offset)
{
    if ((dir % 2 == 0 && val + offset >= CHUNK_SIZE) ||
        (dir % 2 == 1 && val - offset < 0))
        return (VEC3(i32)){ -1, -1, -1 };

    offset = -offset;

    switch (dir)
    {
        case 0:
            return (VEC3(i32)){ val - offset, x, y };
        case 1:
            return (VEC3(i32)){ val + offset, x, y };
        case 2:
            return (VEC3(i32)){ x, val - offset, y };
        case 3:
            return (VEC3(i32)){ x, val + offset, y };
        case 4:
            return (VEC3(i32)){ x, y, val - offset };
        default:
            return (VEC3(i32)){ x, y, val + offset };
    }
}

static inline bool is_bit_set(i64 n, int b)
{
    return n & (1lu << b);
}
static inline bool is_bit_unset(i64 n, int b)
{
    return !is_bit_set(n, b);
}

static inline void set_bit(i64* n, int b)
{
    (*n) |= (1lu << b);
}

static inline void set_border_bits(i64 faces[BLOCK_COUNT][6][CHUNK_SIZE * CHUNK_SIZE], Chunk* c, BlockType b, int dir, int x, int y, MAP(ChunkPos, ChunkPtr)* w)
{
    static VEC3(i32) dirs[6] = {
        { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
    };

    VEC3(i32) prev_chunk_pos = VEC3_SUB(c->pos, dirs[dir / 2]);
    ChunkPtr* prev_chunk = MAP_GET_T(ChunkPos, ChunkPtr, *w, prev_chunk_pos);
    if (prev_chunk != NULL)
    {
        VEC3(i32) dir_prev = get_dir_vector(x, y, dir, CHUNK_SIZE - 1, 0);
        if ((*prev_chunk)->blocks[CHUNK_IDX(dir_prev.x, dir_prev.y, dir_prev.z)].type != BLK_AIR)
            set_bit(&faces[b][dir][CHUNK_IDX_2D(x, y)], 0);
    }

    VEC3(i32) next_chunk_pos = VEC3_ADD(c->pos, dirs[dir / 2]);
    ChunkPtr* next_chunk = MAP_GET_T(ChunkPos, ChunkPtr, *w, next_chunk_pos);
    if (next_chunk != NULL)
    {
        VEC3(i32) dir_next = get_dir_vector(x, y, dir, 0, 0);
        if ((*next_chunk)->blocks[CHUNK_IDX(dir_next.x, dir_next.y, dir_next.z)].type != BLK_AIR)
            set_bit(&faces[b][dir][CHUNK_IDX_2D(x, y)], CHUNK_SIZE + 1);
    }
}

static void build_bit_masks(i64 faces[BLOCK_COUNT][6][CHUNK_SIZE * CHUNK_SIZE], Chunk* c, MAP(ChunkPos, ChunkPtr)* w)
{
    for (BlockType b = 0; b < BLOCK_COUNT; b++)
        if (b != BLK_AIR)
            for (int dir = 0; dir < 6; dir++)
                for (int y = 0; y < CHUNK_SIZE; y++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    set_border_bits(faces, c, b, dir, x, y, w);

                    for (int k = 0; k < CHUNK_SIZE; k++)
                    {
                        VEC3(i32) vec_dir = get_dir_vector(x, y, dir, k, 0);
                        VEC3(i32) vec_dir_next = get_dir_vector(x, y, dir, k, 1);

                        if (c->blocks[CHUNK_IDX(vec_dir.x, vec_dir.y, vec_dir.z)].type == b)
                        {
                            if (vec_dir_next.x == -1)
                            {
                                if (dir % 2 == 1 && is_bit_unset(faces[b][dir][CHUNK_IDX_2D(x, y)], 0))
                                    set_bit(&faces[b][dir][CHUNK_IDX_2D(x, y)], k + 1);
                                else if (dir % 2 == 0 && is_bit_unset(faces[b][dir][CHUNK_IDX_2D(x, y)], CHUNK_SIZE + 1))
                                    set_bit(&faces[b][dir][CHUNK_IDX_2D(x, y)], k + 1);
                            }
                            else if (c->blocks[CHUNK_IDX(vec_dir_next.x, vec_dir_next.y, vec_dir_next.z)].type == BLK_AIR)
                                set_bit(&faces[b][dir][CHUNK_IDX_2D(x, y)], k + 1);
                        }
                    }
                }
}

static void build_faces(i64 faces[BLOCK_COUNT][6][CHUNK_SIZE * CHUNK_SIZE], Chunk* c, VECTOR(Face)* buf)
{
    VEC3(i32) base = VEC3_CAST(i32, VEC3_MUL(c->pos, CHUNK_SIZE));

    for (BlockType b = 0; b < BLOCK_COUNT; b++)
    if (b != BLK_AIR)
    {
        for (int dir = 0; dir < 6; dir++)
        for (int depth = 0; depth < CHUNK_SIZE; depth++)
        for (int v = 0; v < CHUNK_SIZE; v++)
        for (int u = 0; u < CHUNK_SIZE; u++)
        {
            VEC3(i32) pos = get_dir_vector(u, v, dir, depth, 0);
            if (c->blocks[CHUNK_IDX(pos.x, pos.y, pos.z)].type != b)
                continue;
            if (is_bit_unset(faces[b][dir][CHUNK_IDX_2D(u, v)], depth + 1))
                continue;

            int w = 1;
            for (; u + w < CHUNK_SIZE; w++)
                if (is_bit_unset(faces[b][dir][CHUNK_IDX_2D(u + w, v)], depth + 1))
                    break;

            int h = 1;
            for (; v + h < CHUNK_SIZE; h++)
            {
                bool valid = true;
                for (int k = 0; k < w && valid; k++)
                    if (is_bit_unset(faces[b][dir][CHUNK_IDX_2D(u + k, v + h)], depth + 1))
                        valid = false;

                if (!valid)
                    break;
            }

            for (int dv = 0; dv < h; dv++)
            for (int du = 0; du < w; du++)
                faces[b][dir][CHUNK_IDX_2D(u + du, v + dv)] &= ~(1lu << (depth + 1));

            VEC3(i32) scale = get_dir_vector(w, h, dir, 1, 0);
            Face f = {
                .face_id = dir,
                .pos = VEC3_ADD(pos, base),
                .texture_id = face_texture_resolve(&BlockFaces[b][dir], 0, 0, 0),
                .scale = scale,
            };
            VECTOR_PUSH_BACK(*buf, f);
        }
    }
}

void chunk_to_faces(Chunk* c, MAP(ChunkPos, ChunkPtr)* w, VECTOR(Face)* buf)
{
    TracyZone(ctx, "chunk_to_face");

    i64 faces[BLOCK_COUNT][6][CHUNK_SIZE * CHUNK_SIZE] = { 0 };

    build_bit_masks(faces, c, w);

    build_faces(faces, c, buf);

    TracyZoneEnd(ctx);
}
