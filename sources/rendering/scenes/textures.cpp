#include <glm/glm.hpp>
#include <utils/files/load.hpp>
#include <nuklear.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "textures.h"

// Соотношение сторон экрана
extern float g_screen_aspect;
// UI (Nuklear) контекст
extern nk_context* g_nk_context;

namespace scenes
{
    Textures::Textures()
        : projection_(glm::mat4(1.0f))
        , transforms_{glm::mat4(1.0f),glm::mat4(1.0f)}
        , positions_{glm::vec3(-0.75f, 0.0f, 0.0f),glm::vec3(0.75f, 0.0f, 0.0f)}
        , scales_{glm::vec3(0.5f),glm::vec3(0.5f)}
        , angles_{0.0f, 0.0f}
    {}

    Textures::~Textures() = default;

    /**
     * Загрузка шейдеров, геометрии, текстур
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Textures::load()
    {
        // Шейдеры
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/textures/shaders/base.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/textures/shaders/base.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{"transform", "projection", "texture_sampler"});
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
            geometry_ = utils::gl::Geometry<Vertex>(vertices, indices, attributes);
        }

        // Текстуры
        {
            // Подготовка к загрузке текстурных данных
            int width = 0, height = 0, channels = 0;
            unsigned char* bytes;
            stbi_set_flip_vertically_on_load(true);

            // Загрузить данные из файлов, создать OpenGL ресурсы, удалить данные
            bytes = stbi_load("../content/textures/images/box_1.png", &width, &height, &channels, STBI_rgb_alpha);
            textures_[0] = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);

            bytes = stbi_load("../content/textures/images/box_2.png", &width, &height, &channels, STBI_rgb_alpha);
            textures_[1] = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);
        }

        // Проверка доступности ресурсов
        assert(shader_.ready());
        assert(geometry_.ready());
        assert(textures_[0].ready());
        assert(textures_[1].ready());
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Textures::unload()
    {
        shader_.unload();
        geometry_.unload();
        textures_[0].unload();
        textures_[1].unload();
    }

    /**
     * В данном примере задаются 2 трансформации
     * @param delta Временная дельта кадра
     */
    void Textures::update([[maybe_unused]] float delta)
    {
        // Ортогональная проекция (с учетом соотношения экрана)
        projection_ = glm::ortho(-2.0f * g_screen_aspect,2.0f * g_screen_aspect,-2.0f,2.0f);

        // Трансформация для первой отрисовки
        transforms_[0] =
                glm::translate(glm::mat4(1.0f),positions_[0]) *
                glm::rotate(glm::mat4(1.0f), glm::radians(angles_[0]),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),scales_[0]);

        // Трансформация для второй отрисовки
        transforms_[1] =
                glm::translate(glm::mat4(1.0f),positions_[1]) *
                glm::rotate(glm::mat4(1.0f), glm::radians(angles_[1]),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),scales_[1]);
    }

    /**
     * В данном примере отсутствует свой UI
     * @param delta Временная дельта кадра
     */
    void Textures::update_ui([[maybe_unused]] float delta)
    {}

    /**
     * Рисование сцены
     * В данном примере рисуются 2 квадрата с разными текстурами
     */
    void Textures::render()
    {
        // Использовать шейдер
        glUseProgram(shader_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_.vao_id());

        // Привязка текстур к текстурным "слотам"
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures_[0].id());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures_[1].id());

        // Нарисовать геометрию используя проекцию, трансформацию 1 и текстуру 1
        glUniformMatrix4fv(shader_.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(transforms_[0]));
        glUniform1i(shader_.uniform_locations().texture, 0);
        glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);

        // Нарисовать геометрию используя проекцию, трансформацию 2 и текстуру 2
        glUniformMatrix4fv(shader_.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(transforms_[1]));
        glUniform1i(shader_.uniform_locations().texture, 1);
        glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /**
     * Имя примера
     * @return Строка с именем
     */
    const char *Textures::name()
    {
        return "Textures";
    }
}