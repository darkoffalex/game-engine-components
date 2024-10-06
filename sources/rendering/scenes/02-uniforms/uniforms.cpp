#include <glm/glm.hpp>
#include <utils/files/load.hpp>
#include <utils/geometry/generate.hpp>
#include <imgui.h>

#include "uniforms.h"

// Соотношение сторон экрана
extern float g_screen_aspect;

namespace scenes
{
    Uniforms::Uniforms()
        : projection_(glm::mat4(1.0f))
        , transforms_{glm::mat4(1.0f),glm::mat4(1.0f)}
        , positions_{glm::vec3(-0.75f, 0.0f, 0.0f),glm::vec3(0.75f, 0.0f, 0.0f)}
        , scales_{glm::vec3(0.5f),glm::vec3(0.5f)}
        , angles_{0.0f, 0.0f}
    {}

    Uniforms::~Uniforms() = default;

    /**
     * Загрузка шейдеров, геометрии
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Uniforms::load()
    {
        // Загрузить исходные коды шейдеров
        const std::unordered_map<GLuint, std::string> shader_sources = {
                {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/uniforms/base.vert")},
                {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/uniforms/base.frag")}
        };

        // Создать OpenGL ресурс шейдера из исходников
        shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{"transform", "projection"});

        // Данные о геометрии (обычно загружается из файлов)
        using utils::geometry::EAttrBit;
        std::vector<GLuint> indices = {};
        std::vector<Vertex> vertices = utils::geometry::gen_quad<Vertex>(
                2.0f,
                EAttrBit::POSITION|EAttrBit::COLOR,
                offsetof(Vertex, position),0,0,
                offsetof(Vertex, color),
                &indices);

        // Описание атрибутов шейдера
        const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                {0,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, position)},
                {1,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(Vertex, color)}
        };

        // Создать OpenGL ресурс геометрических буферов из данных
        geometry_ = utils::gl::Geometry<Vertex>(vertices, indices, attributes);

        assert(shader_.ready());
        assert(geometry_.ready());
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Uniforms::unload()
    {
        shader_.unload();
        geometry_.unload();
    }

    /**
     * В данном примере задаются 2 трансформации а также проекция
     * @param delta Временная дельта кадра
     */
    void Uniforms::update([[maybe_unused]] float delta)
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
     * В данном примере есть диалоговые окна параметров
     * @param delta Временная дельта кадра
     */
    void Uniforms::update_ui([[maybe_unused]] float delta)
    {
        for(unsigned i = 0; i < 2; ++i)
        {
            if(ImGui::Begin(i == 0 ? "Object 1" : "Object 2", nullptr))
            {
                ImGui::SliderFloat3("Position", (float*)&(positions_[i]), -2.0f, 2.0f);
                ImGui::SliderFloat("Rotation", &angles_[i], -360.0f, 360.0f);

                ImGui::SetWindowSize({220.0f, 100.0f}, ImGuiCond_Once);
                ImGui::SetNextWindowPos({0, ImGui::GetWindowPos().y + 100.0f }, ImGuiCond_Once);
            }
            ImGui::End();
        }
    }

    /**
     * Рисование сцены
     * В данном примере рисуются 2 квадрата с разными цветами вершин
     */
    void Uniforms::render()
    {
        // Использовать шейдер
        glUseProgram(shader_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_.vao_id());

        // Нарисовать геометрию используя проекцию и трансформацию 1
        glUniformMatrix4fv(shader_.uniforms().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniforms().transform, 1, GL_FALSE, glm::value_ptr(transforms_[0]));
        glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);

        // Нарисовать геометрию используя проекцию и трансформацию 2
        glUniformMatrix4fv(shader_.uniforms().projection, 1, GL_FALSE, glm::value_ptr(projection_));
        glUniformMatrix4fv(shader_.uniforms().transform, 1, GL_FALSE, glm::value_ptr(transforms_[1]));
        glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);
    }

    /**
     * Имя примера
     * @return Строка с именем
     */
    const char *Uniforms::name()
    {
        return "Uniforms";
    }
}