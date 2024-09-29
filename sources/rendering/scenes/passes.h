#pragma once

#include <utils/gl/shader.hpp>
#include <utils/gl/geometry.hpp>
#include <utils/gl/frame-buffer.hpp>

#include "base.h"

namespace scenes
{
    /**
     * Пример отображения треугольника в два прохода
     * Первый проход пишет информацию в первичный экранный буфер, второй выводит результат в конечный буфер
     */
    class Passes : public Base
    {
    public:
        /**
         * Описание одиночной вершины первого прохода
         * В первом проходе нам требуется только положение и цвет
         */
        struct VertexPrimary
        {
            glm::vec3 position;
            glm::vec3 color;
        };

        /**
         * Описание одиночной вершины второго прохода
         * Во втором проходе нам требуется читать текстуру кадра первого прохода (нужны UV координаты)
         */
        struct VertexSecondary
        {
            glm::vec3 position;
            glm::vec2 uv;
        };

        /**
         * Идентификатор uniform второго первого прохода
         */
        struct ShaderUniformsPrimary
        {};

        /**
         * Идентификатор uniform переменных первого прохода
         * Для второго прохода нам необходимо передавать в шейдер текстуру кадра первого прохода
         */
        struct ShaderUniformsSecondary
        {
            GLint frame_texture;
        };

    public:
        Passes();
        ~Passes() override;

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
         * Отслеживание изменения разрешения во время обновления
         * @param delta Временная дельта кадра
         */
        void update(float delta) override;

        /**
         * В данном примере есть возможность управления разрешением экрана первичного буфера
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
        /**
         * Событие смены разрешения
         * Может быть вызвано как изменением размера она так и изменением масштабирования
         */
        void on_resolution_change();

    private:
        // Ресурсы
        utils::gl::Shader<ShaderUniformsPrimary, GLint> shader_primary_;
        utils::gl::Shader<ShaderUniformsSecondary, GLint> shader_secondary_;
        utils::gl::Geometry<VertexPrimary> geometry_primary_;
        utils::gl::Geometry<VertexSecondary> geometry_secondary_;
        utils::gl::FrameBuffer frame_buffer_primary_;

        // Настройки разрешения и масштабирования
        int prev_width_, prev_height_, prev_scale_index_;
        int scale_index_;
        std::vector<float> scales_;
        std::vector<const char*> scale_names_;

        // Имеет смысл приостановить рендеринг пока фрейм-буфер не доступен
        // Фрейм-буфер может быть пере-создан в результате смены размеров итогового экрана (окна)
        bool render_;

        // Использовать фильтрацию текстуры кадрового буфера
        bool filter_;

        // Текущее разрешение
        std::string resolution_;
    };
}