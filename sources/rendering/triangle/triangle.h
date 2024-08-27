#pragma once

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"

namespace triangle
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
     * Описание набора uniform-переменных
     * В данном примере отсутствуют
     */
    struct ShaderUniforms
    {};

    /**
     * Загрузка необходимых для рендеринга ресурсов
     */
    void load();

    /**
     * Выгрузка ресурсов (когда более не нужны)
     */
    void unload();

    /**
     * Обновление данных (например, после ввода)
     * @param delta
     */
    void update(float delta);

    /**
     * Рисование
     */
    void render();
}