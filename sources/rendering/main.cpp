#include <iostream>
#include <chrono>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Примеры
#include "01-triangle/triangle.h"
#include "02-uniforms/uniforms.h"

// Соотношение сторон экрана
float g_screen_aspect = 1.0f;

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

    // Получить соотношение сторон
    int width = 0;
    int height = 0;
    glfwGetWindowSize(window, &width, &height);
    g_screen_aspect = (float)width / (float)height;

    // Загрузка OpenGL функций (GLAD)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Загрузить необходимые ресурсы
    try
    {
        triangle::load();
        uniforms::load();
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }

    // Время предыдущего кадра
    auto previous_frame = std::chrono::high_resolution_clock::now();

    // Покуда окно не должно быть закрыто
    while(!glfwWindowShouldClose(window))
    {
        // Разница между временем текущего и прошлого кадра
        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float>(now - previous_frame).count();
        previous_frame = now;

        // Проверка ввода (оконная система)
        check_input(window);

        // Обновление данных
        triangle::update(delta);
        uniforms::update(delta);

        // Задать цвет очистки буфера
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Очистка буфера
        glClear(GL_COLOR_BUFFER_BIT);

        // Рендеринг
        //triangle::render();
        uniforms::render();

        // Смена буферов, опрос оконных событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Выгрузить все OpenGL ресурсы
    triangle::unload();
    uniforms::unload();

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
    g_screen_aspect = (float)width / (float)height;
}