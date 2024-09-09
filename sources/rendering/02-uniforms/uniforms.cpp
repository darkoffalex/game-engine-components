#include <nuklear.h>

#include "uniforms.h"
#include "utils/files/load.hpp"

// Соотношение сторон экрана
extern float g_screen_aspect;
// UI (Nuklear) контекст
extern nk_context* g_nk_context;

namespace uniforms
{
    // Ресурсы
    utils::gl::Shader<ShaderUniforms, GLint> g_shader;
    utils::gl::Geometry<Vertex> g_geometry;

    // Трансформация вершин
    glm::mat4 g_projection = glm::mat4(1.0f);
    glm::mat4 g_transform_1 = glm::mat4(1.0f);
    glm::mat4 g_transform_2 = glm::mat4(1.0f);

    glm::vec3 g_position_1 = glm::vec3(-0.75f, 0.0f, 0.0f);
    glm::vec3 g_position_2 = glm::vec3(0.75f, 0.0f, 0.0f);
    glm::vec3 g_scale_1 = glm::vec3(0.5f);
    glm::vec3 g_scale_2 = glm::vec3(0.5f);
    float g_angle_1 = 0.0f;
    float g_angle_2 = 0.0f;

    /**
     * Загрузка необходимых для рендеринга ресурсов
     */
    void load()
    {
        // Загрузить исходные коды шейдеров
        const std::unordered_map<GLuint, std::string> shader_sources = {
                {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/uniforms/shaders/base.vert")},
                {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/uniforms/shaders/base.frag")}
        };

        // Создать OpenGL ресурс шейдера из исходников
        g_shader = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{"transform", "projection"});

        // Данные о геометрии (хардкод, обычно загружается из файлов)
        const std::vector<GLuint> indices = {0,1,2, 2,3,0};
        const std::vector<Vertex> vertices = {
                {{-1.0f, -1.0f, 0.0f},{1.0f, 0.0f,0.0f}},
                {{-1.0f, 1.0f, 0.0f},{0.0f, 1.0f,0.0f}},
                {{1.0f, 1.0f, 0.0f},{0.0f, 0.0f,1.0f}},
                {{1.0f, -1.0f, 0.0f},{1.0f, 1.0f,0.0f}},
        };

        // Описание атрибутов шейдера
        const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                {0,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, position)},
                {1,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, color)}
        };

        // Создать OpenGL ресурс геометрических буферов из данных
        g_geometry = utils::gl::Geometry<Vertex>(vertices, indices, attributes);

        assert(g_shader.initialized());
        assert(g_geometry.initialized());
    }

    /**
     * Обновление данных (например, после ввода)
     * @param delta
     */
    void update([[maybe_unused]] float delta)
    {
        // Менять угол со временем
        //g_angle_1 += delta * 20.0f;
        //g_angle_2 -= delta * 20.0f;

        // Ортогональная проекция (с учетом соотношения экрана)
        g_projection = glm::ortho(-2.0f * g_screen_aspect,2.0f * g_screen_aspect,-2.0f,2.0f);

        // Трансформация для первой отрисовки
        g_transform_1 =
                glm::translate(glm::mat4(1.0f),g_position_1) *
                glm::rotate(glm::mat4(1.0f), glm::radians(g_angle_1),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),glm::vec3(0.5f));

        // Трансформация для второй отрисовки
        g_transform_2 =
                glm::translate(glm::mat4(1.0f),g_position_2) *
                glm::rotate(glm::mat4(1.0f), glm::radians(g_angle_2),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),glm::vec3(0.5f));
    }

    /**
     * Рисование
     */
    void render()
    {
        // Использовать шейдер
        glUseProgram(g_shader.id());
        // Привязать геометрию
        glBindVertexArray(g_geometry.vao_id());

        // Нарисовать геометрию используя проекцию и трансформацию 1
        glUniformMatrix4fv(g_shader.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(g_projection));
        glUniformMatrix4fv(g_shader.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(g_transform_1));
        glDrawElements(GL_TRIANGLES, g_geometry.index_count(), GL_UNSIGNED_INT, nullptr);

        // Нарисовать геометрию используя проекцию и трансформацию 2
        glUniformMatrix4fv(g_shader.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(g_projection));
        glUniformMatrix4fv(g_shader.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(g_transform_2));
        glDrawElements(GL_TRIANGLES, g_geometry.index_count(), GL_UNSIGNED_INT, nullptr);
    }

    /**
     * Выгрузка ресурсов (когда более не нужны)
     */
    void unload()
    {
        g_shader.~Shader();
        g_geometry.~Geometry();
    }

    /**
     * Обновление и обработка UI элементов (Nuklear)
     */
    void ui_update()
    {
        // Диалог настроек 1
        if (nk_begin(
                g_nk_context,
                "Object 1",
                nk_rect(10, 170, 200, 200),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            // Положение - заголовок
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_label(g_nk_context, "Position:", NK_TEXT_LEFT);

            // Положение - поля
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "X", -10.0f, &g_position_1.x, 10.0f, 0.1f, 0.1f);
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "Y", -10.0f, &g_position_1.y, 10.0f, 0.1f, 0.1f);

            // Поворот - заголовок
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_label(g_nk_context, "Rotation:", NK_TEXT_LEFT);

            // Поворот - поле
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "Angle Z", -360.0f, &g_angle_1, 360.0f, 0.15f, 0.15f);
        }
        nk_end(g_nk_context);

        // Диалог настроек 2
        if (nk_begin(
                g_nk_context,
                "Object 2",
                nk_rect(10, 380, 200, 200),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            // Положение - заголовок
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_label(g_nk_context, "Position:", NK_TEXT_LEFT);

            // Положение - поля
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "X", -10.0f, &g_position_2.x, 10.0f, 0.1f, 0.1f);
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "Y", -10.0f, &g_position_2.y, 10.0f, 0.1f, 0.1f);

            // Поворот - заголовок
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_label(g_nk_context, "Rotation:", NK_TEXT_LEFT);

            // Поворот - поле
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_property_float(g_nk_context, "Angle Z", -360.0f, &g_angle_2, 360.0f, 0.15f, 0.15f);
        }
        nk_end(g_nk_context);
    }
}