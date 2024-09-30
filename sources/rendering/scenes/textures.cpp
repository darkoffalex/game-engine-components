#include <glm/glm.hpp>
#include <utils/files/load.hpp>
#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "textures.h"

// Соотношение сторон экрана
extern float g_screen_aspect;

namespace scenes
{
    Textures::Textures()
        : projection_(glm::mat4(1.0f))
        , transforms_{glm::mat4(1.0f),glm::mat4(1.0f)}
        , uv_transform_{glm::mat3(1.0f),glm::mat3(1.0f)}
        , uv_offsets_{glm::vec2(0.0f, 0.0f),glm::vec2(0.0f, 0.0f)}
        , uv_scales_{glm::vec2(1.0f),glm::vec3(1.0f)}
        , uv_angles_{0.0f, 0.0f}
        , uv_wrap_{GL_REPEAT, GL_REPEAT}
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
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/textures/base.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/textures/base.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{
                "transform",
                "projection",
                "texture_mapping",
                "texture_sampler"
            });
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
            bytes = stbi_load("../content/textures/box_1.png", &width, &height, &channels, STBI_rgb_alpha);
            textures_[0] = utils::gl::Texture2D(bytes, width, height, GL_LINEAR_MIPMAP_LINEAR, utils::gl::Texture2D::EColorSpace::RGB_ALPHA, true);
            stbi_image_free(bytes);

            bytes = stbi_load("../content/textures/box_2.png", &width, &height, &channels, STBI_rgb_alpha);
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
                glm::translate(glm::mat4(1.0f),glm::vec3(-1.25f, 0.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),glm::vec3(1.0f));

        // Трансформация для второй отрисовки
        transforms_[1] =
                glm::translate(glm::mat4(1.0f),glm::vec3(1.25f, 0.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),glm::vec3(0.0f,0.0f,1.0f)) *
                glm::scale(glm::mat4(1.0f),glm::vec3(1.0f));

        // Трансформация UV координат для первой отрисовки
        for(unsigned i = 0; i < 2; i++)
        {
            glm::mat3 uv_translate = glm::mat3(
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(uv_offsets_[i].x, uv_offsets_[i].y,1.0f));

            glm::mat3 uv_rotate = glm::mat3(
                    glm::vec3(glm::cos(glm::radians(uv_angles_[i])), glm::sin(glm::radians(uv_angles_[i])), 0.0f),
                    glm::vec3(-glm::sin(glm::radians(uv_angles_[i])), glm::cos(glm::radians(uv_angles_[i])), 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

            glm::mat3 uv_scale = glm::mat3(
                    glm::vec3(uv_scales_[i].x, 0.0f, 0.0f),
                    glm::vec3(0.0f, uv_scales_[i].y, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

            uv_transform_[i] =  uv_rotate * uv_translate * uv_scale;
        }
    }

    /**
     * В данном примере отсутствует свой UI
     * @param delta Временная дельта кадра
     */
    void Textures::update_ui([[maybe_unused]] float delta)
    {
        for(unsigned i = 0; i < 2; ++i)
        {
            ImGui::Begin(i == 0 ? "Object 1" : "Object 2", nullptr);
            ImGui::SliderFloat2("UV Offset", (float*)&(uv_offsets_[i]), -2.0f, 2.0f);
            ImGui::SliderFloat2("UV Scale", (float*)&(uv_scales_[i]), -2.0f, 2.0f);
            ImGui::SliderFloat("Angle", (float*)&(uv_angles_[i]), -360.0f, 360.0f);

            ImGui::SetWindowSize({220.0f, 100.0f}, ImGuiCond_Once);
            ImGui::SetNextWindowPos({0, ImGui::GetWindowPos().y + 100.0f }, ImGuiCond_Once);
            ImGui::End();
        }
    }

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
        // Задать матрицу проекцию (для всех draw call'ов)
        glUniformMatrix4fv(shader_.uniforms().projection, 1, GL_FALSE, glm::value_ptr(projection_));

        for(unsigned i = 0; i < 2; ++i)
        {
            // Привязка текстур к текстурным "слотам" + установка правил wrap'инга
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures_[i].id());
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uv_wrap_[i]);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uv_wrap_[i]);

            // Нарисовать геометрию используя трансформацию положений вершин и UV координат
            glUniformMatrix4fv(shader_.uniforms().transform, 1, GL_FALSE, glm::value_ptr(transforms_[i]));
            glUniformMatrix3fv(shader_.uniforms().texture_mapping, 1, GL_FALSE, glm::value_ptr(uv_transform_[i]));
            glUniform1i(shader_.uniforms().texture, (GLint)i);
            glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);

            // Сброс
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
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