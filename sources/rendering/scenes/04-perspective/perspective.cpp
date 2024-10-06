#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <utils/files/load.hpp>
#include <utils/geometry/generate.hpp>
#include <stb_image.h>

#include "perspective.h"

// Соотношение сторон экрана
extern float g_screen_aspect;
// Включен ли UI (свободная камера доступна вы отключенном UI)
extern bool g_use_ui;
// Управление
extern bool g_key_forward;
extern bool g_key_backward;
extern bool g_key_left;
extern bool g_key_right;
extern bool g_key_downward;
extern bool g_key_upward;
extern float g_mouse_delta_x;
extern float g_mouse_delta_y;

namespace scenes
{
    Perspective::Perspective()
            : projection_(glm::mat4(1.0f))
            , view_(glm::mat4(1.0f))
            , model_{glm::mat4(1.0f),glm::mat4(1.0f)}
            , camera_pos_(glm::vec3(0.0f, 0.0f, 2.5f))
            , object_pos_{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)}
            , object_scale_{glm::vec3(1.0f),glm::vec3(1.0f, 1.0f, 1.0f)}
            , object_rotation_{glm::vec3(0.0f),glm::vec3(0.0f)}
            , z_far_(100.0f)
            , z_near_(0.1f)
            , fov_(45.0f)
            , cam_yaw_(0.0f)
            , cam_pitch_(0.0f)
            , cam_sensitivity_(0.1f)
            , cam_speed_(1.0f)
            , cam_movement_(0.0f)
    {}

    Perspective::~Perspective() = default;

    /**
     * Загрузка шейдеров, геометрии
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Perspective::load()
    {
        // Шейдеры
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/perspective/base.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/perspective/base.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{
                    "model",
                    "view",
                    "projection",
                    "texture_sampler"
            });
        }

        // Геометрия
        {
            // Данные о геометрии (хардкод, обычно загружается из файлов)
            using utils::geometry::EAttrBit;
            std::vector<GLuint> indices = {};
            std::vector<Vertex> vertices = utils::geometry::gen_cube<Vertex>(
                    1.0f,
                    EAttrBit::POSITION|EAttrBit::UV,
                    offsetof(Vertex, position),
                    offsetof(Vertex, uv),0,0,
                    &indices);

            // Описание атрибутов шейдера
            const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                    {0, 3, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, position)},
                    {1, 2, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, uv)}
            };

            // Создать OpenGL ресурс геометрических буферов из данных
            geometry_ = utils::gl::Geometry<Vertex>(vertices, indices, attributes);
        }

        // Текстуры
        {
            // Подготовка к загрузке текстурных данных
            int width = 0, height = 0, channels = 0;
            unsigned char* bytes;
            stbi_set_flip_vertically_on_load(true);

            // Загрузить данные из файлов, создать OpenGL ресурсы, удалить данные
            bytes = stbi_load("../content/textures/box_1.png", &width, &height, &channels, STBI_rgb_alpha);
            texture_ = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);
        }

        // Проверка доступности ресурсов
        assert(shader_.ready());
        assert(geometry_.ready());
        assert(texture_.ready());
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Perspective::unload()
    {
        shader_.unload();
        geometry_.unload();
    }

    /**
     * В данном примере задаются 2 трансформации а также проекция
     * @param delta Временная дельта кадра
     */
    void Perspective::update([[maybe_unused]] float delta)
    {
        // Вращение объектов
        object_rotation_[0].y += delta * 45.0f;
        object_rotation_[1].y -= delta * 45.0f;

        // Управление свободной камерой
        if(!g_use_ui)
        {
            cam_pitch_ -= (g_mouse_delta_y * cam_sensitivity_);
            cam_yaw_ -= (g_mouse_delta_x * cam_sensitivity_);

            cam_movement_ = {};
            if(g_key_forward) cam_movement_.z = -1.0f;
            else if(g_key_backward) cam_movement_.z = 1.0f;
            if (g_key_left) cam_movement_.x = -1.0f;
            else if(g_key_right) cam_movement_.x = 1.0f;
            if (g_key_upward) cam_movement_.y = 1.0f;
            else if(g_key_downward) cam_movement_.y = -1.0f;
        }

        // Перспективная проекция (с учетом соотношения экрана)
        // Окончательное преобразование точек в NCD пространство
        projection_ = glm::perspective(fov_, g_screen_aspect, z_near_, z_far_);

        // Камера
        {
            // Поворот камеры
            glm::mat4 cam_rotation =
                    glm::rotate(glm::mat4(1.0f), glm::radians(cam_yaw_),glm::vec3(0.0f,1.0f,0.0f)) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(cam_pitch_),glm::vec3(1.0f,0.0f,0.0f));

            // Учесть поворот камеры при движении.
            // Игнорируем ось Y при получении "повернутого" вектора движения.
            // Добавляем ость Y отдельно (в независимость от поворота она всегда должна быть направлена вертикально)
            glm::vec2 h = glm::vec2(cam_movement_.x, cam_movement_.z);
            glm::vec3 d = glm::length2(h) > 0.0f ? glm::normalize(glm::vec3(h.x, 0.0f, h.y)) : glm::vec3(0.0f);
            glm::vec4 d_rot = cam_rotation * glm::vec4(d, 0.0f);
            camera_pos_ += (glm::vec3(d_rot.x, d_rot.y + cam_movement_.y, d_rot.z) * cam_speed_ * delta);

            // Смещение камеры
            glm::mat4 cam_translate = glm::translate(glm::mat4(1.0f), camera_pos_);

            // Инверсия (матрица вида будет конвертировать глобальные координаты в пространство камеры)
            view_ = glm::inverse(cam_translate * cam_rotation);
        }


        // Матрицы моделей для двух объектов
        for(unsigned i = 0; i < 2; i++)
        {
            model_[i] =
                    glm::translate(glm::mat4(1.0f),object_pos_[i]) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(object_rotation_[i].x),glm::vec3(1.0f,0.0f,0.0f)) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(object_rotation_[i].y),glm::vec3(0.0f,1.0f,0.0f)) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(object_rotation_[i].z),glm::vec3(0.0f,0.0f,1.0f)) *
                    glm::scale(glm::mat4(1.0f),object_scale_[i]);
        }

    }

    /**
     * В данном примере есть диалоговые окна параметров
     * @param delta Временная дельта кадра
     */
    void Perspective::update_ui([[maybe_unused]] float delta)
    {}

    /**
     * Рисование сцены
     * В данном примере рисуются 2 квадрата с разными цветами вершин
     */
    void Perspective::render()
    {
        // Считать передние грани заданными по часовой стрелке
        glFrontFace(GL_CW);
        // Отбрасывать задние грани
        glEnable(GL_CULL_FACE);
        // Включить тест глубины
        glEnable(GL_DEPTH_TEST);

        // Использовать шейдер
        glUseProgram(shader_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_.vao_id());

        // Задать матрицу проекции и вида (для всех draw call'ов)
        glUniformMatrix4fv(shader_.uniforms().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniforms().view, 1, GL_FALSE, glm::value_ptr(view_));

        // Привязка текстур к текстурным "слотам"
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_.id());

        for(auto &m : model_)
        {
            // Нарисовать геометрию используя матрицу модели и текстуру
            glUniformMatrix4fv(shader_.uniforms().model, 1, GL_FALSE, glm::value_ptr(m));
            glUniform1i(shader_.uniforms().texture, 0);
            glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);
        }

        // Сброс
        glActiveTexture(GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /**
     * Имя примера
     * @return Строка с именем
     */
    const char *Perspective::name()
    {
        return "Perspective";
    }
}