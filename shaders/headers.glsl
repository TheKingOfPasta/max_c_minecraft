layout(std140, binding = BINDING_SIMULATION) uniform SimulationBlock {
    float dt;

    uint simulation_ubo;

    vec3 cam_pos;

    float cam_pitch;
    float cam_yaw;
} s;
