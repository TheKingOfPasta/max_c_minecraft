#include "mesh_gen.h"

#include "opengl/tracy.h"
#include "utils/container.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

static const VEC3(i32) dirs[6] = {
    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
};

// match shader's lookup
static const int faceVerts[6][4][3] = {
    { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } }, // +X
    { { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 }, { 0, 0, 0 } }, // -X
    { { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 }, { 0, 1, 0 } }, // +Y
    { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } }, // -Y
    { { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 }, { 0, 0, 1 } }, // +Z
    { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } }, // -Z
};

static inline bool is_world_block_solid(VEC3(i32) wpos, MAP(ChunkPos, ChunkPtr) * w)
{
    VEC3(i32) ch_pos = VEC3_RBS(wpos, CHUNK_SHIFT);
    ChunkPtr* p = MAP_GET_T(ChunkPos, ChunkPtr, *w, ch_pos);
    if (!p)
        return false;
    VEC3(i32) bpos = VEC3_AND(wpos, CHUNK_SIZE - 1);
    return (*p)->blocks[CHUNK_IDX(bpos.x, bpos.y, bpos.z)].type != BLK_AIR;
}

static inline int vertex_ao(int f, int v, VEC3(i32) wpos, MAP(ChunkPos, ChunkPtr) * w)
{
    VEC3(i32) n = dirs[f];
    const int* fv = faceVerts[f][v];

    VEC3(i32) t1, t2;
    if (n.x != 0)
    {
        t1 = (VEC3(i32)){ 0, fv[1] * 2 - 1, 0 };
        t2 = (VEC3(i32)){ 0, 0, fv[2] * 2 - 1 };
    }
    else if (n.y != 0)
    {
        t1 = (VEC3(i32)){ fv[0] * 2 - 1, 0, 0 };
        t2 = (VEC3(i32)){ 0, 0, fv[2] * 2 - 1 };
    }
    else
    {
        t1 = (VEC3(i32)){ fv[0] * 2 - 1, 0, 0 };
        t2 = (VEC3(i32)){ 0, fv[1] * 2 - 1, 0 };
    }

    VEC3(i32) base = VEC3_ADD(wpos, n);
    VEC3(i32) side1 = VEC3_ADD(base, t1);
    VEC3(i32) side2 = VEC3_ADD(base, t2);
    VEC3(i32) corner = VEC3_ADD(side1, t2);

    bool s1 = is_world_block_solid(side1, w);
    bool s2 = is_world_block_solid(side2, w);
    bool co = is_world_block_solid(corner, w);

    if (s1 && s2)
        return 0;
    return 3 - s1 - s2 - co;
}

static inline void emit_face(int f, const BlockType b, VEC3(i32) wpos, VECTOR(Face) * buf,
                             [[maybe_unused]] MAP(ChunkPos, ChunkPtr) * w, VEC3(i32) scale)
{
    /*int ao0 = vertex_ao(f, 0, wpos, w);
    int ao1 = vertex_ao(f, 1, wpos, w);
    int ao2 = vertex_ao(f, 2, wpos, w);
    int ao3 = vertex_ao(f, 3, wpos, w);
    int flip = (ao0 + ao2 < ao1 + ao3) ? 1 : 0;*/
    VECTOR_PUSH_BACK(
        *buf,
        ((Face){
            .face_id = f,
            .texture_id = face_texture_resolve(&BlockFaces[b][f], 0, 0, 0),
            .pos = wpos,
            .scale = scale,
        }));
}

/*static inline void border_block_faces(VEC3(i32) lpos, Chunk* c, Chunk** neighbours, VEC3(i32) base,
                                      VECTOR(Face) * buf, MAP(ChunkPos, ChunkPtr) * w)
{
    const Block* b = &c->blocks[CHUNK_IDX(lpos.x, lpos.y, lpos.z)];
    if (b->type == BLK_AIR)
        return;

    VEC3(i32) wpos = VEC3_ADD(base, lpos);

    for (int f = 0; f < 6; f++)
    {
        VEC3(i32) nv = VEC3_ADD(lpos, dirs[f]);
        if ((unsigned)nv.x < CHUNK_SIZE && (unsigned)nv.y < CHUNK_SIZE
            && (unsigned)nv.z < CHUNK_SIZE)
        {
            if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                continue;
        }
        else
        {
            if (!neighbours[f]
                || neighbours[f]
                        ->blocks[CHUNK_IDX(nv.x & (CHUNK_SIZE - 1), nv.y & (CHUNK_SIZE - 1),
                                           nv.z & (CHUNK_SIZE - 1))]
                        .type
                    != BLK_AIR)
                continue;
        }

        emit_face(f, b, wpos, buf, w);
    }
}*/

static inline VEC3(i32) get_dir_vector(int x, int y, int dir, int val, int offset)
{
    offset = -offset;

    if ((dir % 2 == 0 && (val - offset < 0 || val - offset >= CHUNK_SIZE)) ||
        (dir % 2 == 1 && (val + offset < 0 || val + offset >= CHUNK_SIZE)))
        return (VEC3(i32)){ -1, -1, -1 };

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

void chunk_to_faces(Chunk* c, [[maybe_unused]] MAP(ChunkPos, ChunkPtr) * w, VECTOR(Face) * buf)
{
    TracyZone(ctx, "chunk_to_face");

    VEC3(i32) base = VEC3_CAST(i32, VEC3_MUL(c->pos, CHUNK_SIZE));

    i32 faces[BLOCK_COUNT][6][CHUNK_SIZE * CHUNK_SIZE] = { 0 };

    for (BlockType b = 0; b < BLOCK_COUNT; b++)
        if (b != BLK_AIR)
            for (int dir = 0; dir < 6; dir++)
                for (int y = 0; y < CHUNK_SIZE; y++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    /*VEC3(i32) vec_dir_prev = get_dir_vector(x, y, dir, 0);
                    VEC3(i32) prev_pos = VEC3_SUB(vec_dir_prev, dirs[dir]);
                    ChunkPtr* chunk_prev = MAP_GET_T(ChunkPos, ChunkPtr, (*w), (VEC3_SUB(c->pos, dirs[dir])));

                    if (chunk_prev && (*chunk_prev)->blocks[CHUNK_IDX(prev_pos.x, prev_pos.y, prev_pos.z)].type == b)
                        faces[b][dir][CHUNK_IDX_2D(x, y)] |= 1 << 0;

                    VEC3(i32) vec_dir_next = get_dir_vector(x, y, dir, CHUNK_SIZE - 1);
                    VEC3(i32) next_pos = VEC3_ADD(vec_dir_next, dirs[dir]);
                    ChunkPtr* chunk_next = MAP_GET_T(ChunkPos, ChunkPtr, (*w), (VEC3_ADD(c->pos, dirs[dir])));

                    if (chunk_next && (*chunk_next)->blocks[CHUNK_IDX(next_pos.x, next_pos.y, next_pos.z)].type == b)
                        faces[b][dir][CHUNK_IDX_2D(x, y)] |= 1lu << (CHUNK_SIZE + 1);*/

                    for (int k = 0; k < CHUNK_SIZE; k++)
                    {
                        VEC3(i32) vec_dir = get_dir_vector(x, y, dir, k, 0);
                        VEC3(i32) vec_dir_next = get_dir_vector(x, y, dir, k, 1);

                        if (c->blocks[CHUNK_IDX(vec_dir.x, vec_dir.y, vec_dir.z)].type == b &&
                            (vec_dir_next.x == -1 || c->blocks[CHUNK_IDX(vec_dir_next.x, vec_dir_next.y, vec_dir_next.z)].type == BLK_AIR))
                        {
                            faces[b][dir][CHUNK_IDX_2D(x, y)] |= (1 << k);
                        }
                    }
                }

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
            if (!(faces[b][dir][CHUNK_IDX_2D(u, v)] & (1 << depth)))
                continue;

            int w = 1;
            for (; u + w < CHUNK_SIZE; w++)
                if (!(faces[b][dir][CHUNK_IDX_2D(u + w, v)] & (1 << depth)))
                    break;

            int h = 1;
            for (; v + h < CHUNK_SIZE; h++)
            {
                bool valid = true;
                for (int k = 0; k < w && valid; k++)
                    if (!(faces[b][dir][CHUNK_IDX_2D(u + k, v + h)] & (1 << depth)))
                        valid = false;

                if (!valid)
                    break;
            }

            for (int dv = 0; dv < h; dv++)
            for (int du = 0; du < w; du++)
                faces[b][dir][CHUNK_IDX_2D(u + du, v + dv)] &= ~(1 << depth);

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

    /*for (int z = 1; z < CHUNK_SIZE - 1; z++)
        for (int y = 1; y < CHUNK_SIZE - 1; y++)
            for (int x = 1; x < CHUNK_SIZE - 1; x++)
            {
                const Block* b = &c->blocks[CHUNK_IDX(x, y, z)];
                if (b->type == BLK_AIR)
                    continue;

                VEC3(i32) lpos = { x, y, z };
                VEC3(i32) wpos = VEC3_ADD(base, lpos);
                for (int f = 0; f < 6; f++)
                {
                    VEC3(i32) nv = VEC3_ADD(lpos, dirs[f]);
                    if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                        continue;
                    emit_face(f, b, wpos, buf, w);
                }
            }

    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            border_block_faces((VEC3(i32)){ 0, y, z }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ CHUNK_SIZE - 1, y, z }, c, nb, base, buf, w);
        }
    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces((VEC3(i32)){ x, 0, z }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ x, CHUNK_SIZE - 1, z }, c, nb, base, buf, w);
        }
    for (int y = 1; y < CHUNK_SIZE - 1; y++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces((VEC3(i32)){ x, y, 0 }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ x, y, CHUNK_SIZE - 1 }, c, nb, base, buf, w);
        }*/

    TracyZoneEnd(ctx);
}
