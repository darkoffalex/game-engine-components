#include <iostream>
#include <chrono>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Интерфейс ImGUI
#include <imgui.h>
#include "gui/imgui_impl_glfw.h"
#include "gui/imgui_impl_opengl3.h"

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
// Индекс текущей активной сцены
size_t g_scene_index = 0;

// UI (ImGUI) контекст
ImGuiContext* g_gui_context = nullptr;

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
 * Инициализация UI (imGUI)
 * @param window Окно GLFW
 */
void init_ui(GLFWwindow* window);

/**
 * Обновление UI (ImGUI)
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

    // Инициализация UI (ImGUI)
    init_ui(window);

    // Список сцен
    g_scenes.push_back(new scenes::Triangle());
    g_scenes.push_back(new scenes::Uniforms());
    g_scenes.push_back(new scenes::Textures());
    g_scenes.push_back(new scenes::Perspective());
    g_scenes.push_back(new scenes::Passes());
    g_scenes.push_back(new scenes::Lighting());

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
            // Начало кадра ImGUI
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Подготовка и обработка общих UI элементов (ImGUI)
            update_ui();

            // Подготовка и обработка UI элементов активного примера (ImGUI)
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

            // Рендеринг UI элементов (ImGUI)
            if(g_use_ui)
            {
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    // Завершить работу с ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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
 * Инициализация UI (ImGUI)
 * @param window Окно GLFW
 */
void init_ui([[maybe_unused]] GLFWwindow* window)
{
    IMGUI_CHECKVERSION();

    // Контекст
    g_gui_context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // Стилизация ImGui (тема "Cinder")
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize        = ImVec2( 160, 20 );
    style.FramePadding         = ImVec2( 4, 2 );
    style.ItemSpacing          = ImVec2( 6, 2 );
    style.ItemInnerSpacing     = ImVec2( 6, 4 );
    style.Alpha                = 0.95f;
    style.WindowRounding       = 4.0f;
    style.FrameRounding        = 2.0f;
    style.IndentSpacing        = 6.0f;
    style.ItemInnerSpacing     = ImVec2( 2, 4 );
    style.ColumnsMinSpacing    = 50.0f;
    style.GrabMinSize          = 14.0f;
    style.GrabRounding         = 16.0f;
    style.ScrollbarSize        = 12.0f;
    style.ScrollbarRounding    = 16.0f;
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
//    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);

    // Инициализация для GLFW/OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

/**
 * Обновление UI (ImGUI)
 */
void update_ui()
{
    ImGui::Begin("Settings", nullptr);
    ImGui::SetWindowPos({0.0f, 0.0f}, ImGuiCond_Once);
    ImGui::SetWindowSize({150.0f, 100.0f}, ImGuiCond_Once);
    ImGui::Text("FPS: %s", g_fps_str.c_str());

    if(ImGui::BeginCombo("Scene", g_scenes[g_scene_index]->name()))
    {
        for(unsigned i = 0; i < g_scenes.size(); i++)
        {
            bool is_selected = g_scene_index == i;
            if(ImGui::Selectable(g_scenes[i]->name(), is_selected)) g_scene_index = i;
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SetNextWindowPos({0, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() }, ImGuiCond_Once);
    ImGui::End();
}
