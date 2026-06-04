# MAX_C_MINECRAFT

- [X] Block enum

- [X] Face instantiation
    - [X] In vert ( I have the code )
    - [X] In main
    - [X] Add texture id

- [X] Chunk
    - [X] Base struct
    - [X] 1 opengl buffer
    - [X] Gen mesh

- [X] Textures
    - [X] Map texture to block
    - [X] Load texture
    - [X] Put them in opengl texture_array
    - [X] use them in frag

- [X] World
    - [X] hashmap
    - [ ] export
    - [ ] import

- [ ] Player
    - [X] Camera Movement
    - [ ] Place and Break plock
        - [ ] raycast in voxel space ( I have the code )
        - [ ] re upload buffer
    - [ ] Collision

- [ ] terrain Gen
    - [ ] generate on demand
    - [ ] not on cpu gen
    - [ ] simple height map terrain
    - [ ] proper layered noise
    - [ ] generate structure
    - [ ] biome
    - [ ] proper structure for noise layer
    - [ ] use rastringin function for island

- Visual effect
    - [ ] ambient occlusion

Memory allocator chunks

- Optimizations
    - [ ] minimize data transfer CPU <=> GPU
        - [ ] Face instances : Pack every face instance into a 32 bit number : 5 * 3 bit for pos, 3 bit ? for the face orientation,  the rest for the block type
    - [ ] Frustrum culling
    - [ ] Face culling CPU wise
    - [ ] LOD

Add Fabrizium



