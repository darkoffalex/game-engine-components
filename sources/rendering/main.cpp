#include <iostream>
#include <chrono>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#define NK_MAX_VERTEX_BUFFER (512 * 1024)
#define NK_MAX_ELEMENT_BUFFER (128 * 1024)

// Интерфейс nuklear
#include <nuklear.h>
#include "gui/nuklear_glfw_gl4.h"

// Примеры
#include "01-triangle/triangle.h"
#include "02-uniforms/uniforms.h"

// Экран
float g_screen_aspect = 1.0f;
int g_screen_width = 0;
int g_screen_height = 0;

// FPS
int g_fps = 0;
std::string g_fps_str;
float g_fps_until_next_update = 1.0f;

// Наименования примеров (в списке)
std::vector<const char*> g_example_names = {
        "triangle",
        "uniforms"
};

// Функции рендеринга примеров
std::vector<std::function<void()>> g_example_render_callbacks = {
        [](){triangle::render();},
        [](){uniforms::render();}
};

// Текущий активный пример
size_t g_example_selected_index = 0;

// UI (Nuklear) контекст
nk_context* g_nk_context = nullptr;

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rendering", nullptr, nullptr);
    if(!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Сделать окно основным, задать обработчик смены размеров, отключить в-синхронизацию
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);

    // Получить соотношение сторон
    glfwGetWindowSize(window, &g_screen_width, &g_screen_height);
    g_screen_aspect = (float)g_screen_width / (float)g_screen_height;

    // Загрузка OpenGL функций (GLAD)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Nuklear
    g_nk_context = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, NK_MAX_VERTEX_BUFFER, NK_MAX_ELEMENT_BUFFER);
    {
        nk_font_atlas *atlas = nullptr;
        nk_glfw3_font_stash_begin(&atlas);
        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
        nk_glfw3_font_stash_end();
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        /*nk_style_set_font(ctx, &droid->handle);*/
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
        // Опрос оконных событий
        glfwPollEvents();

        // Разница между временем текущего и прошлого кадра
        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float>(now - previous_frame).count();
        previous_frame = now;

        // Обновить счетчик кадров
        g_fps_until_next_update -= delta;
        if(g_fps_until_next_update <= 0)
        {
            g_fps = static_cast<int>(1.0f / delta);
            g_fps_str = std::to_string(g_fps);
            g_fps_until_next_update = 1.0f;
        }

        // Подготовка и обработка UI элементов (Nuklear)
        nk_glfw3_new_frame();
        if (nk_begin(
                g_nk_context,
                "Settings",
                nk_rect(10, 10, 200, 150),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            // FPS
            nk_layout_row_dynamic(g_nk_context, 20, 2);
            nk_label(g_nk_context, "FPS: ", NK_TEXT_LEFT);
            nk_label(g_nk_context, g_fps_str.c_str(), NK_TEXT_LEFT);

            // Заголовок - примеры
            nk_layout_row_dynamic(g_nk_context, 20, 1);
            nk_label(g_nk_context, "Example:", NK_TEXT_LEFT);

            // Селектор примера
            nk_layout_row_dynamic(g_nk_context, 25, 1);
            g_example_selected_index = nk_combo(
                    g_nk_context, g_example_names.data(),
                    (int)g_example_names.size(),
                    (int)g_example_selected_index,
                    20,
                    nk_vec2(150,150));
        }
        nk_end(g_nk_context);

        // Проверка ввода (оконная система)
        check_input(window);

        // Обновление данных
        triangle::update(delta);
        uniforms::update(delta);

        // Сброс всех состояний и очистка экрана
        glViewport(0, 0, g_screen_width, g_screen_height);
        glScissor(0, 0, g_screen_width, g_screen_height);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рендеринг выбранного примера
        g_example_render_callbacks[g_example_selected_index]();

        // Рендеринг UI элементов (Nuklear)
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        // Смена буферов
        glfwSwapBuffers(window);
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
    g_screen_width = width;
    g_screen_height = height;
    g_screen_aspect = (float)width / (float)height;
}