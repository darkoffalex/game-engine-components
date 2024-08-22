#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils/gl/shader.hpp"

/**
 * Вызывается GLFW при смене размеров целевого фрейм-буфера
 * @param window Окно
 * @param width Ширина
 * @param height Высота
 */
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * Обработка пользовательского ввода
 * @param window Окно
 */
void check_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

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

    // Покуда окно не должно быть закрыто
    while(!glfwWindowShouldClose(window))
    {
        // Проверка ввода
        check_input(window);

        // Рендеринг
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Смена буферов, опрос оконных событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Завершить работу с GLFW
    glfwTerminate();
    return 0;
}