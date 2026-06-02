// Don't change order, padding matters
layout(std140, binding = BINDING_SIMULATION) uniform SimulationBlock {
    float dt;

    uint simulation_ubo;

    float cam_pitch;
    float cam_yaw;

    vec3 cam_pos;
} s;
