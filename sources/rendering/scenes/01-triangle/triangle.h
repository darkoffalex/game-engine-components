#pragma once

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"

#include "../scene.h"

namespace scenes
{
    /**
     * Пример отображения треугольника
     * Простейший пример отображающий простейшую геометрию (треугольник)
     */
    class Triangle : public Scene
    {
    public:
        /**
         * Описание одиночной вершины
         * В данном примере используется только положение и цвет
         */
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
        };

        /**
         * Идентификатор uniform переменных в шейдере
         * Используется при инициализации шейдера (в данном примере нет uniform переменных)
         */
        struct ShaderUniforms
        {};

    public:
        Triangle() = default;
        ~Triangle() override;

        /**
         * Загрузка шейдеров, геометрии
         * Геометрия в данном примере hardcoded, остальное загружается из файлов
         */
        void load() override;

        /**
         * Выгрузка всех использованных ресурсов граф. API
         */
        void unload() override;

        /**
         * В данном примере отсутствует обновление данных
         * @param delta Временная дельта кадра
         */
        void update(float delta) override;

        /**
         * В данном примере отсутствует свой UI
         * @param delta Временная дельта кадра
         */
        void update_ui(float delta) override;

        /**
         * Рисование сцены
         * В данном примере всего один вызов отрисовки
         */
        void render() override;

        /**
         * Имя примера
         * @return Строка с именем
         */
        const char* name() override;

    protected:
        // Ресурсы
        utils::gl::Shader<ShaderUniforms, GLint> shader_;
        utils::gl::Geometry<Vertex> geometry_;
    };
}