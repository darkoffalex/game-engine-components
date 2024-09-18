#include <glm/glm.hpp>
#include <utils/files/load.hpp>

#include "triangle.h"

namespace scenes
{
    Triangle::~Triangle() = default;

    /**
     * Загрузка шейдеров, геометрии
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Triangle::load()
    {
        // Загрузить исходные коды шейдеров
        const std::unordered_map<GLuint, std::string> shader_sources = {
                {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/triangle/shaders/base.vert")},
                {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/triangle/shaders/base.frag")}
        };

        // Создать OpenGL ресурс шейдера из исходников
        shader_ = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{});

        // Данные о геометрии (хардкод, обычно загружается из файлов)
        const std::vector<GLuint> indices = {0,1,2};
        const std::vector<Vertex> vertices = {
                {{-1.0f, -1.0f, 0.0f},{1.0f, 0.0f,0.0f}},
                {{0.0f, 1.0f, 0.0f},{0.0f, 1.0f,0.0f}},
                {{1.0f, -1.0f, 0.0f},{0.0f, 0.0f,1.0f}},
        };

        // Описание атрибутов шейдера
        const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                {0,3,GL_FLOAT, GL_FALSE, offsetof(Vertex, position)},
                {1,3,GL_FLOAT, GL_FALSE, offsetof(Vertex, color)}
        };

        // Создать OpenGL ресурс геометрических буферов из данных
        geometry_ =  utils::gl::Geometry<Vertex>(vertices, indices, attributes);

        assert(shader_.ready());
        assert(geometry_.ready());
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Triangle::unload()
    {
        shader_.unload();
        shader_.unload();
    }

    /**
     * В данном примере отсутствует обновление данных
     * @param delta Временная дельта кадра
     */
    void Triangle::update([[maybe_unused]] float delta)
    {}

    /**
     * В данном примере отсутствует свой UI
     * @param delta Временная дельта кадра
     */
    void Triangle::update_ui([[maybe_unused]] float delta)
    {}

    /**
     * Рисование сцены
     * В данном примере всего один вызов отрисовки
     */
    void Triangle::render()
    {
        // Использовать шейдер
        glUseProgram(shader_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_.vao_id());
        // Нарисовать геометрию
        glDrawElements(GL_TRIANGLES, geometry_.index_count(), GL_UNSIGNED_INT, nullptr);
    }

    /**
     * Имя примера
     * @return Строка с именем
     */
    const char *Triangle::name()
    {
        return "Triangle";
    }
}