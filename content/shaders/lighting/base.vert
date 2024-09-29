#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec2 uv;
    vec3 pos;
    vec3 normal;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);

    vs_out.uv = uv;
    vs_out.pos = (model * vec4(position, 1.0)).xyz;
    vs_out.normal = (model * vec4(normal, 0.0)).xyz;
}