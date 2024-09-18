#pragma once

#include <utils/gl/shader.hpp>
#include <utils/gl/geometry.hpp>
#include <utils/gl/texture-2d.hpp>

#include "base.h"

namespace scenes
{
    /**
     * Пример загрузки и использования текстур
     */
    class Textures : public Base
    {
    public:
        /**
         * Описание одиночной вершины
         * В данном примере используется только положение и UV координаты
         */
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 uv;
        };

        /**
         * Идентификатор uniform переменных в шейдере
         * Используется при инициализации шейдера
         */
        struct ShaderUniforms
        {
            GLint transform;
            GLint projection;
            GLint texture;
        };

    public:
        Textures();
        ~Textures() override;

        /**
         * Загрузка шейдеров, геометрии, текстур
         * Геометрия в данном примере hardcoded, остальное загружается из файлов
         */
        void load() override;

        /**
         * Выгрузка всех использованных ресурсов граф. API
         */
        void unload() override;

        /**
         * В данном примере задаются 2 трансформации а также проекция
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
         * В данном примере рисуются 2 квадрата с разными текстурами
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
        utils::gl::Texture2D textures_[2];

        // Матрицы для передачи шейдеру
        glm::mat4 projection_;
        glm::mat4 transforms_[2];

        // Положения, масштабы и прочие данные для построения/обновления матриц
        glm::vec3 positions_[2];
        glm::vec3 scales_[2];
        float angles_[2];
    };
}