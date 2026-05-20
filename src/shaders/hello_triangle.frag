#version 460 core

layout (location = 0) out vec4 out_color;

layout (location = 0) in v2f {
    vec3 color;
} In;

void main() {
    out_color = vec4(In.color, 1.0);
}
