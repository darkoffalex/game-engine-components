#version 420 core

layout (location = 0) out vec4 color;

uniform sampler2D texture_sampler;

in VS_OUT {
    vec2 uv;
} fs_in;

void main()
{
    color = texture(texture_sampler, fs_in.uv);
}