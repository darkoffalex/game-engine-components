#version 420 core

layout (location = 0) out vec4 color;

#define MAX_LIGHT_SOURCES 2
#define LIGHT_TYPE_AMBIENT 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define LIGHT_TYPE_DIRECTIONAL 3

uniform vec3   light_positions[MAX_LIGHT_SOURCES];
uniform vec3   light_directions[MAX_LIGHT_SOURCES];
uniform vec3   light_colors[MAX_LIGHT_SOURCES];
uniform uint   light_types[MAX_LIGHT_SOURCES];
uniform float  light_fall_offs[MAX_LIGHT_SOURCES];
uniform float  light_hot_spots[MAX_LIGHT_SOURCES];
uniform uint   light_count;

in VS_OUT {
    vec2 uv;
    vec3 pos;
    vec3 normal;
} fs_in;

void main()
{
    // Итоговая совещенность
    vec3 illumination = vec3(0.0f);

    // Обработка источников света
    for(int i = 0; i < light_count; i++)
    {
        // Интенсивность максимальна по умолчанию и затухание отсутствует
        float intensity = 1.0f;
        float attenuation = 0.0f;

        // Для точечных и ambient источников
        // Получить затухание используя 2 радиуса (привет, Serious Sam!)
        if(light_types[i] == LIGHT_TYPE_POINT || light_types[i] == LIGHT_TYPE_AMBIENT)
        {
            float fal_off = light_fall_offs[i];
            float hot_spot = light_hot_spots[i];
            float len = length(fs_in.pos - light_positions[i]);
            attenuation = clamp(len - hot_spot, 0.0f, len) / clamp(fal_off - hot_spot, 0.01f, fal_off);
        }

        // Точечные истоники и прожекторы
        // Получить интенсивность по углу между падением света и нормалью
        if(light_types[i] == LIGHT_TYPE_POINT || light_types[i] == LIGHT_TYPE_SPOT)
        {
            vec3 l = normalize(light_positions[i] - fs_in.pos);
            vec3 n = normalize(fs_in.normal);
            intensity = clamp(dot(l, n), 0.0f, 1.0f);

            if(light_types[i] == LIGHT_TYPE_SPOT)
            {
                intensity = 0.0f;
                // TODO: Реализовать освещение для прожекторов
            }
        }
        else if(light_types[i] == LIGHT_TYPE_DIRECTIONAL)
        {
            intensity = 0.0f;
            // TODO: Реализовать освещение для направленных источников
        }

        // Итоговая совещенность
        illumination += (clamp(1.0f - attenuation, 0.0f, 1.0f) * intensity * light_colors[i]);
    }

    color = vec4(illumination, 1.0f);
}