#version 460 core

layout(location = 0) out v2f {
    vec4 color;
} Out;

void main() {
    const vec3 position[3] = vec3[3](
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );
    const vec4 color[3] = vec4[3](
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    );
    Out.color = color[gl_VertexID];
    gl_Position = vec4(position[gl_VertexID], 1.0);
}
