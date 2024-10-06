#pragma once

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"
#include "utils/gl/texture-2d.hpp"

#include "../scene.h"

namespace scenes
{
    /**
     * Пример перспективной проекции
     * Отображение вращающихся кубов
     */
    class Perspective : public Scene
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
            GLint model;
            GLint view;
            GLint projection;
            GLint texture;
        };

    public:
        Perspective();
        ~Perspective() override;

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
        utils::gl::Texture2D texture_;

        // Матрицы для преобразования вершин
        glm::mat4 projection_;
        glm::mat4 view_;
        glm::mat4 model_[2];

        // Пространственные параметры объектов
        glm::vec3 camera_pos_;
        glm::vec3 object_pos_[2];
        glm::vec3 object_scale_[2];
        glm::vec3 object_rotation_[2];

        // Параметры для построения матрицы проекции
        GLfloat z_far_, z_near_, fov_;

        // Доп параметры для управления камерой
        GLfloat cam_yaw_, cam_pitch_, cam_sensitivity_, cam_speed_;
        glm::vec3 cam_movement_;
    };
}