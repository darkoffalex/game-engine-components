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
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#define NK_MAX_VERTEX_BUFFER (512 * 1024)
#define NK_MAX_ELEMENT_BUFFER (128 * 1024)

// Интерфейс nuklear
#include <nuklear.h>
#include "gui/nuklear_glfw_gl3.h"

// Примеры
#include "scenes/triangle.h"
#include "scenes/uniforms.h"
#include "scenes/textures.h"
#include "scenes/perspective.h"
#include "scenes/passes.h"
#include "scenes/lighting.h"

// Экран
float g_screen_aspect = 1.0f;
int g_screen_width = 0;
int g_screen_height = 0;

// FPS
int g_fps = 0;
std::string g_fps_str;
float g_fps_until_next_update = 1.0f;

// Список сцен
std::vector<scenes::Base*> g_scenes = {};
// Список имен сцен
std::vector<const char*> g_scene_names = {};
// Индекс текущей активной сцены
size_t g_scene_index = 0;

// UI (Nuklear) контекст
nk_context* g_nk_context = nullptr;
nk_glfw g_glfw = {};

// Использовать UI
bool g_use_ui = true;

// Управление
bool g_key_forward = false;
bool g_key_backward = false;
bool g_key_left = false;
bool g_key_right = false;
bool g_key_downward = false;
bool g_key_upward = false;
float g_mouse_delta_x = 0.0f;
float g_mouse_delta_y = 0.0f;

/**
 * Вызывается GLFW при смене размеров целевого фрейм-буфера
 * @param window Окно
 * @param width Ширина
 * @param height Высота
 */
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height);

/**
 * Обработка событий ввода с клавиатуры
 * @param window Окно
 * @param key Кнопка
 * @param scancode Код символа
 * @param action Тип действия (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
 * @param mods Биты модификации (GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT...)
 */
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

/**
 * Обработка событий перемещения курсора
 * @param window Окно
 * @param x_pos Положение по горизонтали
 * @param y_pos Положение по вертикали
 */
void mouse_pos_callback(GLFWwindow* window, double x_pos, double y_pos);

/**
 * Инициализация UI (Nuklear)
 * @param window Окно GLFW
 */
void init_ui(GLFWwindow* window);

/**
 * Обновление UI (Nuklear)
 */
void update_ui();

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
    glfwSetKeyCallback(window, keyboard_callback);
    glfwSetCursorPosCallback(window, mouse_pos_callback);
    //glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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

    // Инициализация UI (Nuklear)
    init_ui(window);

    // Список сцен
    g_scenes.push_back(new scenes::Triangle());
    g_scenes.push_back(new scenes::Uniforms());
    g_scenes.push_back(new scenes::Textures());
    g_scenes.push_back(new scenes::Perspective());
    g_scenes.push_back(new scenes::Passes());
    g_scenes.push_back(new scenes::Lighting());

    // Названия сцен
    for(auto* s : g_scenes) g_scene_names.push_back(s->name());

    // Загрузить необходимые ресурсы сцен-примеров
    try
    {
        for(auto* s : g_scenes)
        {
            s->load();
        }
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
            g_fps_str = std::to_string(g_fps);
            g_fps = 0;
            g_fps_until_next_update = 1.0f;

            std::string title = "Rendering: (FPS " + g_fps_str + ")";
            glfwSetWindowTitle(window, title.c_str());
        }
        else
        {
            g_fps++;
        }

        // Обновление UI (если нужно)
        if(g_use_ui)
        {
            // Подготовка и обработка общих UI элементов (Nuklear)
            nk_glfw3_new_frame(&g_glfw);
            update_ui();

            // Подготовка и обработка UI элементов активного примера (Nuklear)
            g_scenes[g_scene_index]->update_ui(delta);
        }

        // Обновление данных выбранной сцены
        g_scenes[g_scene_index]->update(delta);

        // Сброс смещения курсора мыши
        g_mouse_delta_x = 0.0f;
        g_mouse_delta_y = 0.0f;

        // Р Е Н Д Е Р И Н Г
        {
            // Сброс всех состояний и очистка экрана
            glViewport(0, 0, g_screen_width, g_screen_height);
            glScissor(0, 0, g_screen_width, g_screen_height);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Рендеринг выбранного примера
            g_scenes[g_scene_index]->render();

            // Рендеринг UI элементов (Nuklear)
            if(g_use_ui)
            {
                nk_glfw3_render(&g_glfw, NK_ANTI_ALIASING_ON, NK_MAX_VERTEX_BUFFER, NK_MAX_ELEMENT_BUFFER);
            }
        }

        // Смена буферов
        glfwSwapBuffers(window);
    }

    // Выгрузить все OpenGL ресурсы сцен
    for(auto* s : g_scenes)
    {
        s->unload();
        delete s;
    }

    // Завершить работу с GLFW
    glfwTerminate();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Обработка событий ввода с клавиатуры
 * @param window Окно
 * @param key Кнопка
 * @param scancode Код символа
 * @param action Тип действия (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
 * @param mods Биты модификации (GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT...)
 */
void keyboard_callback([[maybe_unused]] GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    // Кнопка нажата один раз
    if(action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_U:
            {
                g_use_ui = !g_use_ui;
                glfwSetInputMode(window, GLFW_CURSOR, g_use_ui ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                g_scene_index = glm::clamp<size_t>(g_scene_index + 1, 0, g_scenes.size() - 1);
                break;
            }
            case GLFW_KEY_LEFT:
            {
                g_scene_index = glm::clamp<size_t>(g_scene_index - 1, 0, g_scenes.size() - 1);
                break;
            }
            case GLFW_KEY_ESCAPE:
            {
                glfwSetWindowShouldClose(window, true);
                break;
            }
            case GLFW_KEY_W:
            {
                g_key_forward = true;
                break;
            }
            case GLFW_KEY_S:
            {
                g_key_backward = true;
                break;
            }
            case GLFW_KEY_D:
            {
                g_key_right = true;
                break;
            }
            case GLFW_KEY_A:
            {
                g_key_left = true;
                break;
            }
            case GLFW_KEY_C:
            {
                g_key_downward = true;
                break;
            }
            case GLFW_KEY_SPACE:
            {
                g_key_upward = true;
                break;
            }
            default:
                break;
        }
    }
    else if(action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_W:
            {
                g_key_forward = false;
                break;
            }
            case GLFW_KEY_S:
            {
                g_key_backward = false;
                break;
            }
            case GLFW_KEY_D:
            {
                g_key_right = false;
                break;
            }
            case GLFW_KEY_A:
            {
                g_key_left = false;
                break;
            }
            case GLFW_KEY_C:
            {
                g_key_downward = false;
                break;
            }
            case GLFW_KEY_SPACE:
            {
                g_key_upward = false;
                break;
            }
            default:
                break;
        }
    }
}

/**
 * Обработка событий перемещения курсора
 * @param window Окно
 * @param x_pos Положение по горизонтали
 * @param y_pos Положение по вертикали
 */
void mouse_pos_callback([[maybe_unused]] GLFWwindow *window, double x_pos, double y_pos)
{
    static double prev_x = x_pos;
    static double prev_y = y_pos;
    g_mouse_delta_x = static_cast<float>(x_pos - prev_x);
    g_mouse_delta_y = static_cast<float>(y_pos - prev_y);
    prev_x = x_pos;
    prev_y = y_pos;
}

/**
 * Вызывается GLFW при смене размеров целевого фрейм-буфера
 * @param window Окно
 * @param width Ширина
 * @param height Высота
 */
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, const int width, const int height)
{
    g_screen_width = width;
    g_screen_height = height;
    g_screen_aspect = (float)width / (float)height;
}

/**
 * Инициализация UI (Nuklear)
 * @param window Окно GLFW
 */
void init_ui(GLFWwindow* window)
{
    // Контекст nuklear
    g_nk_context = nk_glfw3_init(&g_glfw, window, NK_GLFW3_INSTALL_CALLBACKS);

    // Шрифты
    nk_font_atlas *atlas = nullptr;
    nk_glfw3_font_stash_begin(&g_glfw, &atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    nk_glfw3_font_stash_end(&g_glfw);

    // Задать шрифт (раскомментировать при необходимости)
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &droid->handle);*/
}

/**
 * Обновление UI (Nuklear)
 */
void update_ui()
{
    // Диалог настроек
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
        nk_label(g_nk_context, "Scene:", NK_TEXT_LEFT);

        // Селектор примера
        nk_layout_row_dynamic(g_nk_context, 25, 1);
        g_scene_index = nk_combo(
                g_nk_context, g_scene_names.data(),
                (int)g_scene_names.size(),
                (int)g_scene_index,
                20,
                nk_vec2(150,150));
    }
    nk_end(g_nk_context);
}
