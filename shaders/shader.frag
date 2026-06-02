out vec4 FragColor;

#define LIGHT_DIR normalize(vec3(0.45, 0.85, 0.30))
#define LIGHT_COLOR vec3(1.0, 0.97, 0.92)
#define AMBIENT_LIGHT vec3(0.18, 0.22, 0.28)

vec3 get_dir()
{
    float x = (gl_FragCoord.x / 1920.0) * 2.0 - 1;
    float y = (gl_FragCoord.y / 1080.0) * 2.0 - 1;

    float aspect = 1920.0 / 1080.0;

    vec3 dir = normalize(vec3(x * aspect, y, 1.0 / tan(radians(60.0) * 0.5)));

    float cosPitch = cos(s.cam_pitch);
    float sinPitch = sin(s.cam_pitch);
    float cosYaw = cos(s.cam_yaw);
    float sinYaw = sin(s.cam_yaw);

    mat3 pitchMat = mat3(1, 0, 0, 0, cosPitch, sinPitch, 0, -sinPitch, cosPitch);

    mat3 yawMat = mat3(cosYaw, 0, -sinYaw, 0, 1, 0, sinYaw, 0, cosYaw);

    return yawMat * pitchMat * dir;
}

vec3 background_sky(vec3 dir)
{
    vec3 skyBottom = vec3(54.0, 98.0, 227.0) / 255.0;
    vec3 skyTop = vec3(168.0, 183.0, 227.0) / 255.0;

    float t = smoothstep(0.0, 1.0, (dir.y + 1.0) * 0.5);
    return mix(skyBottom, skyTop, t);
}

vec3 background(vec3 pos, vec3 dir)
{
    if (dir.y < -0.0001)
    {
        float ground_height = -10;
        float t = (ground_height - pos.y) / dir.y;
        if (t > 0.0)
        {
            vec3 hit = pos + dir * t;

            float grid_size = 42 * 4;
            vec3 grid_col_A = vec3(0.1, 0.1, 0.1);
            vec3 grid_col_B = vec3(0.8, 0.8, 0.8);

            float cx = floor(hit.x / grid_size);
            float cz = floor(hit.z / grid_size);
            vec3 base = mix(grid_col_A, grid_col_B, mod(cx + cz, 2.0));

            float lit = max(dot(vec3(0, 1, 0), LIGHT_DIR), 0.0);
            base *= AMBIENT_LIGHT + LIGHT_COLOR * lit * 0.8;
            float haze = clamp(t / 5000.0, 0.0, 1.0);
            return mix(base, background_sky(dir), haze);
        }
    }

    vec3 col = background_sky(dir);
    float sun = max(dot(dir, LIGHT_DIR), 0.0);
    col += LIGHT_COLOR * pow(sun, 350.0) * 1.2;
    col += LIGHT_COLOR * pow(sun, 8.0) * 0.18;

    return col;
}
void main()
{
    vec3 dir = get_dir();

    vec3 pos_or = s.cam_pos;
    FragColor = vec4(background(pos_or, dir), 1.0);
}
