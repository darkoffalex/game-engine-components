#pragma once

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"

namespace uniforms
{
    /**
     * Описание одной вершины
     */
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    /**
     * Описание набора uniform-переменных.
     * Содержит только одну матрицу трансформации точек
     */
    struct ShaderUniforms
    {
        GLint transform;
        GLint projection;
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