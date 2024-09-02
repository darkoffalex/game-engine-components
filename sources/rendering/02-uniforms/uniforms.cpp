#include "uniforms.h"
#include "utils/files/load.hpp"

namespace uniforms
{
    // Ресурсы
    utils::gl::Shader<ShaderUniforms, GLint> g_shader;
    utils::gl::Geometry<Vertex> g_geometry;

    // Трансформация
    glm::mat4 g_transform = glm::mat4(1.0f);

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
        g_shader = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{"transform"});

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
                {0,3,GL_FLOAT, GL_FALSE, offsetof(Vertex, position)},
                {1,3,GL_FLOAT, GL_FALSE, offsetof(Vertex, color)}
        };

        // Создать OpenGL ресурс геометрических буферов из данных
        g_geometry =  utils::gl::Geometry<Vertex>(vertices, indices, attributes);

        assert(g_shader.initialized());
        assert(g_geometry.initialized());
    }

    /**
     * Обновление данных (например, после ввода)
     * @param delta
     */
    void update([[maybe_unused]] float delta)
    {
        g_transform = glm::scale(glm::mat4(1.0f),glm::vec3(0.5f));
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
        // Передать трансформацию (как uniform переменную)
        glUniformMatrix4fv(g_shader.uniform_locations().transform, 1, GL_FALSE, glm::value_ptr(g_transform));
        // Нарисовать геометрию
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
}