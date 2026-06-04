# MAX_C_MINECRAFT

- [X] Block enum

- [X] Face instantiation
    - [X] In vert ( I have the code )
    - [X] In main
    - [X] Add texture id

- [ ] Chunk
    - [X] Base struct
    - [ ] 1 opengl buffer
    - [ ] Gen mesh

- [X] Textures
    - [X] Map texture to block
    - [X] Load texture
    - [X] Put them in opengl texture_array
    - [X] use them in frag

- [ ] World
    - [ ] hashmap

- [ ] Player
    - [ ] Camera Movement
    - [ ] Place and Break plock
        - [ ] raycast in voxel space ( I have the code )
        - [ ] re upload buffer
    - [ ] Collision

- Make it look good
    - [ ] ambient occlusion

Memory allocator chunks

- Optimizations
    - [ ] minimize data transfer CPU <=> GPU
        - [ ] Face instances : Pack every face instance into a 32 bit number : 5 * 3 bit for pos, 3 bit ? for the face orientation,  the rest for the block type
    - [ ] Frustrum culling
    - [ ] Face culling CPU wise
    - [ ] LOD

Add Fabrizium



