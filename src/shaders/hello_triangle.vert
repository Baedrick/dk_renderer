#version 460 core

layout (location = 0) out v2f {
    vec3 color;
} Out;

void main() {
    const vec3 pos[3] = vec3[3](
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );
    Out.color = pos[gl_VertexID] + vec3(0.5, 0.5, 0.0);
    gl_Position = vec4(pos[gl_VertexID], 1.0);
}
