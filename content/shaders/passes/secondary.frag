#version 420 core

layout (location = 0) out vec4 color;

uniform sampler2D frame_texture;

in VS_OUT {
    vec2 uv;
} fs_in;

void main()
{
    color = texture(frame_texture, fs_in.uv);
}