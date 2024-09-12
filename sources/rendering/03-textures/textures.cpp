#include <nuklear.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "textures.h"
#include "utils/files/load.hpp"

// Соотношение сторон экрана
extern float g_screen_aspect;
// UI (Nuklear) контекст
extern nk_context* g_nk_context;

namespace textures
{
    // Ресурсы
    utils::gl::Shader<ShaderUniforms, GLint> g_shader;
    utils::gl::Geometry<Vertex> g_geometry;
    utils::gl::Texture2D g_texture_1;
    utils::gl::Texture2D g_texture_2;

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
        // Шейдеры
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/textures/shaders/base.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/textures/shaders/base.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            g_shader = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{"transform", "projection", "texture_sampler"});
        }

        // Геометрия
        {
            // Данные о геометрии (хардкод, обычно загружается из файлов)
            const std::vector<GLuint> indices = {0,1,2, 2,3,0};
            const std::vector<Vertex> vertices = {
                    {{-1.0f, -1.0f, 0.0f},{0.0f, 0.0f}},
                    {{-1.0f, 1.0f, 0.0f},{0.0f, 1.0f}},
                    {{1.0f, 1.0f, 0.0f},{1.0f, 1.0f}},
                    {{1.0f, -1.0f, 0.0f},{1.0f, 0.0f}},
            };

            // Описание атрибутов шейдера
            const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                    {0, 3, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, position)},
                    {1, 2, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, uv)}
            };

            // Создать OpenGL ресурс геометрических буферов из данных
            g_geometry = utils::gl::Geometry<Vertex>(vertices, indices, attributes);
        }

        // Текстуры
        {
            // Подготовка к загрузке текстурных данных
            int width = 0, height = 0, channels = 0;
            unsigned char* bytes = nullptr;
            stbi_set_flip_vertically_on_load(true);

            // Загрузить данные из файлов, создать OpenGL ресурсы, удалить данные
            bytes = stbi_load("../content/textures/images/box_1.png", &width, &height, &channels, STBI_rgb_alpha);
            g_texture_1 = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);

            bytes = stbi_load("../content/textures/images/box_2.png", &width, &height, &channels, STBI_rgb_alpha);
            g_texture_2 = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);
        }

        // Проверка доступности ресурсов
        assert(g_shader.initialized());
        assert(g_geometry.initialized());
        assert(g_texture_1.initialized());
        assert(g_texture_2.initialized());
    }

    /**
     * Обновление данных (например, после ввода)
     * @param delta
     */
    void update([[maybe_unused]] float delta)
    {
        // Ортогональная проекция (с учетом соотношения экрана)
        g_projection = glm::ortho(-1.0f * g_screen_aspect,1.0f * g_screen_aspect,-1.0f,1.0f);

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

        // Привязка текстур к текстурным "слотам"
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_texture_1.id());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_texture_2.id());

        // Нарисовать геометрию используя проекцию, трансформацию 1 и текстуру 1
        glUniformMatrix4fv(g_shader.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(g_projection));
        glUniformMatrix4fv(g_shader.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(g_transform_1));
        glUniform1i(g_shader.uniform_locations().texture, 0);
        glDrawElements(GL_TRIANGLES, g_geometry.index_count(), GL_UNSIGNED_INT, nullptr);

        // Нарисовать геометрию используя проекцию, трансформацию 2 и текстуру 2
        glUniformMatrix4fv(g_shader.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(g_projection));
        glUniformMatrix4fv(g_shader.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(g_transform_2));
        glUniform1i(g_shader.uniform_locations().texture, 1);
        glDrawElements(GL_TRIANGLES, g_geometry.index_count(), GL_UNSIGNED_INT, nullptr);
    }

    /**
     * Выгрузка ресурсов (когда более не нужны)
     */
    void unload()
    {
        g_shader.~Shader();
        g_geometry.~Geometry();
        g_texture_1.~Texture2D();
        g_texture_2.~Texture2D();
    }

    /**
     * Обновление и обработка UI элементов (Nuklear)
     */
    void ui_update()
    {
    }
}