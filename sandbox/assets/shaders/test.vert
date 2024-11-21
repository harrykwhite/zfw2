#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in float a_tex_index;
layout (location = 5) in vec2 a_tex_coord;
layout (location = 6) in float a_alpha;

out flat int v_tex_index;
out vec2 v_tex_coord;
out float v_alpha;

uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    float rot_cos = cos(a_rot);
    float rot_sin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);

    v_tex_index = int(a_tex_index);
    v_tex_coord = a_tex_coord;
    v_alpha = a_alpha;
}
