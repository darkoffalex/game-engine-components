#include <glm/glm.hpp>
#include <utils/files/load.hpp>
#include <nuklear.h>
#include <stb_image.h>

#include "perspective.h"

// Соотношение сторон экрана
extern float g_screen_aspect;
// UI (Nuklear) контекст
extern nk_context* g_nk_context;

namespace scenes
{
    Perspective::Perspective()
            : projection_(glm::mat4(1.0f))
            , view_(glm::mat4(1.0f))
            , model_{glm::mat4(1.0f),glm::mat4(1.0f)}
            , camera_pos_(glm::vec3(0.0f, 0.0f, 2.5f))
            , object_pos_{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)}
            , object_scale_{glm::vec3(1.0f),glm::vec3(1.0f)}
            , object_rotation_{glm::vec3(0.0f),glm::vec3(0.0f)}
            , z_far_(100.0f)
            , z_near_(0.1f)
            , fov_(45.0f)
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
            const std::vector<GLuint> indices = {
                    0,1,2, 2,3,0,
                    4,5,6, 6,7,4,
                    8,9,10, 10,11,8,
                    12,13,14, 14,15,12
            };
            const std::vector<Vertex> vertices = {
                    {{-0.5f, -0.5f, 0.5f},{0.0f, 0.0f}},
                    {{-0.5f, 0.5f, 0.5f},{0.0f, 1.0f}},
                    {{0.5f, 0.5f, 0.5f},{1.0f, 1.0f}},
                    {{0.5f, -0.5f, 0.5f},{1.0f, 0.0f}},

                    {{0.5f, -0.5f, 0.5f},{0.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.5f},{0.0f, 1.0f}},
                    {{0.5f, 0.5f, -0.5f},{1.0f, 1.0f}},
                    {{0.5f, -0.5f, -0.5f},{1.0f, 0.0f}},

                    {{0.5f, -0.5f, -0.5f},{0.0f, 0.0f}},
                    {{0.5f, 0.5f, -0.5f},{0.0f, 1.0f}},
                    {{-0.5f, 0.5f, -0.5f},{1.0f, 1.0f}},
                    {{-0.5f, -0.5f, -0.5f},{1.0f, 0.0f}},

                    {{-0.5f, -0.5f, -0.5f},{0.0f, 0.0f}},
                    {{-0.5f, 0.5f, -0.5f},{0.0f, 1.0f}},
                    {{-0.5f, 0.5f, 0.5f},{1.0f, 1.0f}},
                    {{-0.5f, -0.5f, 0.5f},{1.0f, 0.0f}},
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
        object_rotation_[0].y += delta * 45.0f;
        object_rotation_[1].y -= delta * 45.0f;

        // Ортогональная проекция (с учетом соотношения экрана)
        // Окончательное преобразование точек в NCD пространство
        projection_ = glm::perspective(fov_, g_screen_aspect, z_near_, z_far_);

        // Матрица вида
        // Преобразование точек в пространство вида (центр - положение камеры)
        view_ = glm::inverse(glm::translate(glm::mat4(1.0f), camera_pos_));

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
    {
    }

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

        // Использовать шейдер
        glUseProgram(shader_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_.vao_id());

        // Задать матрицу проекцию и вида (для всех draw call'ов)
        glUniformMatrix4fv(shader_.uniform_locations().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniform_locations().view, 1, GL_FALSE, glm::value_ptr(view_));

        // Привязка текстур к текстурным "слотам" + установка правил wrap'инга
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_.id());

        for(auto &m : model_)
        {
            // Нарисовать геометрию используя матрицу модели и текстуру
            glUniformMatrix4fv(shader_.uniform_locations().model, 1, GL_FALSE, glm::value_ptr(m));
            glUniform1i(shader_.uniform_locations().texture, 0);
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