#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 transform;
uniform mat4 projection;
uniform mat3 texture_mapping;

out VS_OUT {
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = projection * transform * vec4(position, 1.0);
    //vs_out.uv = uv;
    vs_out.uv = (texture_mapping * vec3(uv.x, uv.y, 1.0f)).xy;
}