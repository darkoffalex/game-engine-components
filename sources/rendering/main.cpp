#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils/gl/shader.hpp"
#include "utils/gl/geometry.hpp"
#include "utils/files/load.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

struct ShaderUniforms
{};

/**
 * Вызывается GLFW при смене размеров целевого фрейм-буфера
 * @param window Окно
 * @param width Ширина
 * @param height Высота
 */
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height);

/**
 * Обработка пользовательского ввода
 * @param window Окно
 */
void check_input(GLFWwindow* window);

/**
 * Точка входа
 * @param argc Кол-во аргументов
 * @param argv Аргументы
 * @return Код выполнения
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    // Попытка инициализации GLFW
    if(!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        return -1;
    }

    // Подготовка GLFW для работы с OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создать основное окно
    GLFWwindow* window = glfwCreateWindow(800, 600, "Window", nullptr, nullptr);
    if(!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Сделать окно основным и задать обработчик смены размеров
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Загрузка OpenGL функций (GLAD)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Шейдер и геометрия (пустые обертки над OpenGL ресурсами, инициализируются позже)
    utils::gl::Shader<ShaderUniforms, GLint> shader;
    utils::gl::Geometry<Vertex> geometry;

    // Загрузить необходимые ресурсы
    try
    {
        // Загрузить исходные коды шейдеров
        const std::unordered_map<GLuint, std::string> shader_sources = {
                {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/triangle/shaders/base.vert")},
                {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/triangle/shaders/base.frag")}
        };

        // Создать OpenGL ресурс шейдера из исходников
        shader = utils::gl::Shader<ShaderUniforms, GLint>(shader_sources,{});


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
        geometry =  utils::gl::Geometry<Vertex>(vertices, indices, attributes);

    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    // Убедиться в готовности ресурсов
    assert(shader.initialized());
    assert(geometry.initialized());

    // Покуда окно не должно быть закрыто
    while(!glfwWindowShouldClose(window))
    {
        // Проверка ввода
        check_input(window);

        // Задать цвет очистки буфера
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Очистка буфера
        glClear(GL_COLOR_BUFFER_BIT);

        // Использовать шейдер
        glUseProgram(shader.id());
        // Привязать геометрию
        glBindVertexArray(geometry.vao_id());
        // Нарисовать геометрию
        glDrawElements(GL_TRIANGLES, geometry.index_count(), GL_UNSIGNED_INT, nullptr);

        // Смена буферов, опрос оконных событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Выгрузить все OpenGL ресурсы
    shader.~Shader();
    geometry.~Geometry();

    // Завершить работу с GLFW
    glfwTerminate();
    return 0;
}

void check_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}