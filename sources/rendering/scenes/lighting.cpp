#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <utils/files/load.hpp>
#include <nuklear.h>
#include <stb_image.h>

#include "lighting.h"

// Соотношение сторон экрана
extern float g_screen_aspect;
// UI (Nuklear) контекст
extern nk_context* g_nk_context;
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
    const std::vector<const char*> Lighting::light_type_names_ = {
            "Ambient",
            "Point",
            "Spot",
            "Directional"
    };

    Lighting::Lighting()
            : projection_(glm::mat4(1.0f))
            , view_(glm::mat4(1.0f))
            , model_{glm::mat4(1.0f),glm::mat4(1.0f)}
            , camera_pos_(glm::vec3(0.0f, 2.0f, 4.0f))
            , object_pos_{glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.25f, 0.0f)}
            , object_scale_{glm::vec3(10.0f, 0.5f, 10.0f),glm::vec3(1.0f, 1.0f, 1.0f)}
            , object_rotation_{glm::vec3(0.0f),glm::vec3(0.0f)}
            , z_far_(100.0f)
            , z_near_(0.1f)
            , fov_(45.0f)
            , cam_yaw_(0.0f)
            , cam_pitch_(-25.0f)
            , cam_sensitivity_(0.1f)
            , cam_speed_(1.0f)
            , cam_movement_(0.0f)
    {}

    Lighting::~Lighting() = default;

    /**
     * Загрузка шейдеров, геометрии
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Lighting::load()
    {
        // Шейдеры
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/lighting/base.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/lighting/base.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{
                    "model",
                    "view",
                    "projection",

                    "light_positions",
                    "light_colors",
                    "light_directions",
                    "light_types",
                    "light_fall_offs",
                    "light_hot_spots",
                    "light_count"
            });
        }

        // Геометрия
        {
            // Данные о геометрии (хардкод, обычно загружается из файлов)
            const std::vector<GLuint> indices = {
                    0,1,2, 2,3,0,
                    4,5,6, 6,7,4,
                    8,9,10, 10,11,8,
                    12,13,14, 14,15,12,
                    16,17,18, 18,19,16,
                    20,21,22, 22,23,20
            };
            const std::vector<Vertex> vertices = {
                    {{-0.5f, -0.5f, 0.5f},{0.0f, 0.0f},{0.0f, 0.0f, 1.0f}},
                    {{-0.5f, 0.5f, 0.5f},{0.0f, 1.0f},{0.0f, 0.0f, 1.0f}},
                    {{0.5f, 0.5f, 0.5f},{1.0f, 1.0f},{0.0f, 0.0f, 1.0f}},
                    {{0.5f, -0.5f, 0.5f},{1.0f, 0.0f},{0.0f, 0.0f, 1.0f}},

                    {{0.5f, -0.5f, 0.5f},{0.0f, 0.0f},{1.0f, 0.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.5f},{0.0f, 1.0f},{1.0f, 0.0f, 0.0f}},
                    {{0.5f, 0.5f, -0.5f},{1.0f, 1.0f},{1.0f, 0.0f, 0.0f}},
                    {{0.5f, -0.5f, -0.5f},{1.0f, 0.0f},{1.0f, 0.0f, 0.0f}},

                    {{0.5f, -0.5f, -0.5f},{0.0f, 0.0f},{0.0f, 0.0f, -1.0f}},
                    {{0.5f, 0.5f, -0.5f},{0.0f, 1.0f},{0.0f, 0.0f, -1.0f}},
                    {{-0.5f, 0.5f, -0.5f},{1.0f, 1.0f},{0.0f, 0.0f, -1.0f}},
                    {{-0.5f, -0.5f, -0.5f},{1.0f, 0.0f},{0.0f, 0.0f, -1.0f}},

                    {{-0.5f, -0.5f, -0.5f},{0.0f, 0.0f},{-1.0f, 0.0f, 0.0f}},
                    {{-0.5f, 0.5f, -0.5f},{0.0f, 1.0f},{-1.0f, 0.0f, 0.0f}},
                    {{-0.5f, 0.5f, 0.5f},{1.0f, 1.0f},{-1.0f, 0.0f, 0.0f}},
                    {{-0.5f, -0.5f, 0.5f},{1.0f, 0.0f},{-1.0f, 0.0f, 0.0f}},

                    {{-0.5f, 0.5f, 0.5f},{0.0f, 0.0f},{0.0f, 1.0f, 0.0f}},
                    {{-0.5f, 0.5f, -0.5f},{0.0f, 1.0f},{0.0f, 1.0f, 0.0f}},
                    {{0.5f, 0.5f, -0.5f},{1.0f, 1.0f},{0.0f, 1.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.5f},{1.0f, 0.0f},{0.0f, 1.0f, 0.0f}},

                    {{-0.5f, -0.5f, -0.5f},{0.0f, 0.0f},{0.0f, -1.0f, 0.0f}},
                    {{-0.5f, -0.5f, 0.5f},{0.0f, 1.0f},{0.0f, -1.0f, 0.0f}},
                    {{0.5f, -0.5f, 0.5f},{1.0f, 1.0f},{0.0f, -1.0f, 0.0f}},
                    {{0.5f, -0.5f, -0.5f},{1.0f, 0.0f},{0.0f, -1.0f, 0.0f}},
            };

            // Описание атрибутов шейдера
            const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                    {0, 3, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, position)},
                    {1, 2, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, uv)},
                    {2, 3, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, normal)}
            };

            // Создать OpenGL ресурс геометрических буферов из данных
            geometry_ = utils::gl::Geometry<Vertex>(vertices, indices, attributes);
        }

        // Источники света
        {
            light_positions_.emplace_back(-2.0f, 0.5f, 0.0f);
            light_colors_.emplace_back(1.0f, 1.0f, 1.0f);
            light_directions_.emplace_back(0.0f, 0.0f, 0.0f);
            light_types_.emplace_back(0);
            light_hot_spots_.emplace_back(0.5f);
            light_fall_offs_.emplace_back(1.8f);

            light_positions_.emplace_back(2.0f, 0.5f, 0.0f);
            light_colors_.emplace_back(1.0f, 1.0f, 1.0f);
            light_directions_.emplace_back(0.0f, 0.0f, 0.0f);
            light_types_.emplace_back(0);
            light_hot_spots_.emplace_back(0.5f);
            light_fall_offs_.emplace_back(1.8f);
        }

        // Проверка доступности ресурсов
        assert(shader_.ready());
        assert(geometry_.ready());
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Lighting::unload()
    {
        shader_.unload();
        geometry_.unload();
    }

    /**
     * В данном примере задаются 2 трансформации а также проекция
     * @param delta Временная дельта кадра
     */
    void Lighting::update([[maybe_unused]] float delta)
    {
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
    void Lighting::update_ui([[maybe_unused]] float delta)
    {
        for(unsigned i = 0; i < 2; ++i)
        {
            // Добавить диалог настроек
            if (nk_begin(
                    g_nk_context,
                    i == 0 ? "Light 1" : "Light 2",
                    nk_rect(10.0f, 170.0f + (float)(i * 210), 200.0f, 200.0f),
                    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
            {
                // Положение
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_label(g_nk_context, "Position:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_property_float(g_nk_context, "X", -10.0f, &light_positions_[i].x, 10.0f, 0.1f, 0.1f);
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_property_float(g_nk_context, "Y", -10.0f, &light_positions_[i].y, 10.0f, 0.1f, 0.1f);
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_property_float(g_nk_context, "Z", -10.0f, &light_positions_[i].z, 10.0f, 0.1f, 0.1f);

                // Затухание
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_label(g_nk_context, "Attenuation:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_property_float(g_nk_context, "Fall off:", 0.0f, &light_fall_offs_[i], 10.0f, 0.1f, 0.1f);
                nk_layout_row_dynamic(g_nk_context, 20, 1);
                nk_property_float(g_nk_context, "Hot spot:", 0.0f, &light_hot_spots_[i], 10.0f, 0.1f, 0.1f);

                // Тип источника
                nk_layout_row_dynamic(g_nk_context, 20, 2);
                nk_label(g_nk_context, "Type:", NK_TEXT_LEFT);
                light_types_[i] = nk_combo(
                        g_nk_context, (const char**)light_type_names_.data(),
                        (int)light_type_names_.size(),
                        (int)light_types_[i],
                        20,
                        nk_vec2(150,150));

                // Направление
                if(light_types_[i] == (GLuint)ELightType::DIRECTIONAL || light_types_[i] == (GLuint)ELightType::SPOT)
                {
                    nk_layout_row_dynamic(g_nk_context, 20, 1);
                    nk_label(g_nk_context, "Direction:", NK_TEXT_LEFT);
                    nk_layout_row_dynamic(g_nk_context, 20, 1);
                    nk_property_float(g_nk_context, "X", -360.0f, &light_directions_[i].x, 360.0f, 0.15f, 0.15f);
                    nk_layout_row_dynamic(g_nk_context, 20, 1);
                    nk_property_float(g_nk_context, "Y", -360.0f, &light_directions_[i].y, 360.0f, 0.15f, 0.15f);
                    nk_layout_row_dynamic(g_nk_context, 20, 1);
                    nk_property_float(g_nk_context, "Z", -360.0f, &light_directions_[i].z, 360.0f, 0.15f, 0.15f);
                }

                // Цвет
                nk_layout_row_dynamic(g_nk_context, 20, 2);
                nk_label(g_nk_context, "Color:", NK_TEXT_LEFT);
                nk_colorf combo_color2 = {light_colors_[i].r, light_colors_[i].g, light_colors_[i].b, 1.0f};
                if (nk_combo_begin_color(g_nk_context, nk_rgb_cf(combo_color2), nk_vec2(200,400)))
                {
                    nk_layout_row_dynamic(g_nk_context, 120, 1);
                    combo_color2 = nk_color_picker(g_nk_context, combo_color2, NK_RGBA);

                    nk_layout_row_dynamic(g_nk_context, 25, 1);
                    combo_color2.r = nk_propertyf(g_nk_context, "#R:", 0, combo_color2.r, 1.0f, 0.01f,0.005f);
                    combo_color2.g = nk_propertyf(g_nk_context, "#G:", 0, combo_color2.g, 1.0f, 0.01f,0.005f);
                    combo_color2.b = nk_propertyf(g_nk_context, "#B:", 0, combo_color2.b, 1.0f, 0.01f,0.005f);
                    combo_color2.a = nk_propertyf(g_nk_context, "#A:", 0, combo_color2.a, 1.0f, 0.01f,0.005f);

                    light_colors_[i].r = combo_color2.r;
                    light_colors_[i].g = combo_color2.g;
                    light_colors_[i].b = combo_color2.b;

                    nk_combo_end(g_nk_context);
                }
            }
            nk_end(g_nk_context);
        }
    }

    /**
     * Рисование сцены
     * В данном примере рисуются 2 квадрата с разными цветами вершин
     */
    void Lighting::render()
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

        // Передать информацию об источниках освещения
        glUniform3fv(shader_.uniforms().light_positions, (GLsizei)light_positions_.size(), glm::value_ptr(light_positions_[0]));
        glUniform3fv(shader_.uniforms().light_colors, (GLsizei)light_colors_.size(), glm::value_ptr(light_colors_[0]));
        glUniform3fv(shader_.uniforms().light_directions, (GLsizei)light_directions_.size(), glm::value_ptr(light_directions_[0]));
        glUniform1uiv(shader_.uniforms().light_types, (GLsizei)light_types_.size(), light_types_.data());
        glUniform1fv(shader_.uniforms().light_hot_spots, (GLsizei)light_hot_spots_.size(), light_hot_spots_.data());
        glUniform1fv(shader_.uniforms().light_fall_offs, (GLsizei)light_fall_offs_.size(), light_fall_offs_.data());
        glUniform1ui(shader_.uniforms().light_count, (GLuint)light_types_.size());

        for(auto &m : model_)
        {
            // Нарисовать геометрию используя матрицу модели и информацию об источниках света
            glUniformMatrix4fv(shader_.uniforms().model, 1, GL_FALSE, glm::value_ptr(m));
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
    const char *Lighting::name()
    {
        return "Basic lightning";
    }
}