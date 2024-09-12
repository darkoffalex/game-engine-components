#pragma once

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"
#include "utils/gl/texture-2d.hpp"

namespace textures
{
    /**
     * Описание одной вершины
     */
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    /**
     * Описание набора uniform-переменных.
     * Содержит только одну матрицу трансформации точек
     */
    struct ShaderUniforms
    {
        GLint transform;
        GLint projection;
        GLint texture;
    };

    /**
     * Загрузка необходимых для рендеринга ресурсов
     */
    void load();

    /**
     * Обновление данных (например, после ввода)
     * @param delta
     */
    void update(float delta);

    /**
     * Рисование
     */
    void render();

    /**
     * Выгрузка ресурсов (когда более не нужны)
     */
    void unload();

    /**
     * Обновление и обработка UI элементов (Nuklear)
     */
    void ui_update();
}