#include <utils/files/load.hpp>
#include <imgui.h>

#include "passes.h"

// Размеры итогового буфера (окна)
extern int g_screen_width;
extern int g_screen_height;

namespace scenes
{
    Passes::Passes()
            : prev_width_(g_screen_width)
            , prev_height_(g_screen_height)
            , prev_scale_index_(0)
            , scale_index_(0)
            , scales_{1.0f, 0.75f, 0.5f, 0.25f, 0.1f}
            , scale_names_{"100%","75%","50%","25%","10%"}
            , render_(false)
            , filter_(false)
            , resolution_("0x0")
    {}

    Passes::~Passes() = default;

    /**
     * Загрузка шейдеров, геометрии
     * Геометрия в данном примере hardcoded, остальное загружается из файлов
     */
    void Passes::load()
    {
        // Р Е С У Р С Ы  П Р О Х Р Д А  1
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/passes/primary.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/passes/primary.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_primary_ = utils::gl::Shader<ShaderUniformsPrimary, GLint>(shader_sources,{});

            // Данные о геометрии (хардкод, обычно загружается из файлов)
            const std::vector<GLuint> indices = {0,1,2};
            const std::vector<VertexPrimary> vertices = {
                    {{-1.0f, -1.0f, 0.0f},{1.0f, 0.0f,0.0f}},
                    {{0.0f, 1.0f, 0.0f},{0.0f, 1.0f,0.0f}},
                    {{1.0f, -1.0f, 0.0f},{0.0f, 0.0f,1.0f}},
            };

            // Описание атрибутов шейдера
            const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                    {0,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(VertexPrimary, position)},
                    {1,3,GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(VertexPrimary, color)}
            };

            // Создать OpenGL ресурс геометрических буферов из данных
            geometry_primary_ = utils::gl::Geometry<VertexPrimary>(vertices, indices, attributes);
        }

        // Р Е С У Р С Ы  П Р О Х Р Д А  2
        {
            // Загрузить исходные коды шейдеров
            const std::unordered_map<GLuint, std::string> shader_sources = {
                    {GL_VERTEX_SHADER,  utils::files::load_as_text("../content/shaders/passes/secondary.vert")},
                    {GL_FRAGMENT_SHADER, utils::files::load_as_text("../content/shaders/passes/secondary.frag")}
            };

            // Создать OpenGL ресурс шейдера из исходников
            shader_secondary_ = utils::gl::Shader<ShaderUniformsSecondary, GLint>(shader_sources, {"frame_texture"});

            // Данные о геометрии (хардкод, обычно загружается из файлов)
            const std::vector<GLuint> indices = {0,1,2, 2,3,0};
            const std::vector<VertexSecondary> vertices = {
                    {{-1.0f, -1.0f, 0.0f},{0.0f, 0.0f}},
                    {{-1.0f, 1.0f, 0.0f},{0.0f, 1.0f}},
                    {{1.0f, 1.0f, 0.0f},{1.0f, 1.0f}},
                    {{1.0f, -1.0f, 0.0f},{1.0f, 0.0f}},
            };

            // Описание атрибутов шейдера
            const std::vector<utils::gl::VertexAttributeInfo> attributes = {
                    {0, 3, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(VertexSecondary, position)},
                    {1, 2, GL_FLOAT, GL_FALSE, (GLsizeiptr)offsetof(VertexSecondary, uv)}
            };

            // Создать OpenGL ресурс геометрических буферов из данных
            geometry_secondary_ = utils::gl::Geometry<VertexSecondary>(vertices, indices, attributes);
        }

        // Создать первичный кадровый буфер (первого прохода)
        utils::gl::FrameBufferAttachmentInfo color{GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0, GL_NEAREST};
        utils::gl::RenderBufferAttachmentInfo depth_stencil{GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT};
        frame_buffer_primary_ = utils::gl::FrameBuffer(
                g_screen_width,
                g_screen_height,
                {color},
                {depth_stencil});

        // Обновить строчку с разрешением
        resolution_ = std::to_string(frame_buffer_primary_.width()) +
                "x" + std::to_string(frame_buffer_primary_.height());

        assert(shader_primary_.ready());
        assert(geometry_primary_.ready());
        assert(shader_secondary_.ready());
        assert(geometry_secondary_.ready());
        assert(frame_buffer_primary_.ready());

        // Рендеринг возможен
        render_ = true;
    }

    /**
     * Выгрузка всех использованных ресурсов граф. API
     */
    void Passes::unload()
    {
        shader_primary_.unload();
        shader_secondary_.unload();
        geometry_primary_.unload();
        geometry_secondary_.unload();
    }

    /**
     * Отслеживание изменения разрешения во время обновления
     * @param delta Временная дельта кадра
     */
    void Passes::update([[maybe_unused]] float delta)
    {
        if(g_screen_height != prev_height_
           || g_screen_width != prev_width_
           || scale_index_ != prev_scale_index_)
        {
            on_resolution_change();

            prev_width_ = g_screen_width;
            prev_height_ = g_screen_height;
            prev_scale_index_ = scale_index_;
        }
    }

    /**
     * В данном примере есть возможность управления разрешением экрана первичного буфера
     * @param delta Временная дельта кадра
     */
    void Passes::update_ui([[maybe_unused]] float delta)
    {
        if(ImGui::Begin("Frame buffer", nullptr))
        {
            if(ImGui::BeginCombo("Scale", scale_names_[scale_index_]))
            {
                for(size_t i = 0; i < scale_names_.size(); i++)
                {
                    bool is_selected = scale_index_ == i;
                    if(ImGui::Selectable(scale_names_[i], is_selected)) scale_index_ = (int)i;
                    if(is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Text("Resolution: %s", resolution_.c_str());
            ImGui::Checkbox("Filtering", &filter_);

            ImGui::SetWindowSize({200.0f, 100.0f}, ImGuiCond_Once);
            ImGui::SetNextWindowPos({0, ImGui::GetWindowPos().y + 100.0f }, ImGuiCond_Once);
            ImGui::End();
        }
    }

    /**
     * Рисование сцены
     * В данном примере всего один вызов отрисовки
     */
    void Passes::render()
    {
        if(!render_) return;

        // П Р О Х О Д  - 1

        // Использовать первичный кадровый буфер
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_primary_.id());
        // Указать область кадра доступную для отрисовки
        glScissor(0, 0, frame_buffer_primary_.width(), frame_buffer_primary_.height());
        glViewport(0, 0, frame_buffer_primary_.width(), frame_buffer_primary_.height());
        // Очистка буфера
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Использовать шейдер
        glUseProgram(shader_primary_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_primary_.vao_id());
        // Нарисовать геометрию
        glDrawElements(GL_TRIANGLES, geometry_primary_.index_count(), GL_UNSIGNED_INT, nullptr);

        // П Р О Х О Д  - 2

        // Использовать итоговый кадровый буфер
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Указать область кадра доступную для отрисовки
        glScissor(0, 0, g_screen_width, g_screen_height);
        glViewport(0, 0, g_screen_width, g_screen_height);
        // Очистка буфера
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Привязать цветовое вложение кадрового буфера как текстуру к первому слоту
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame_buffer_primary_.attachments_tx()[0]);

        // Фильтрация
        // Только для примера! Устанавливать параметры текстур в каждом кадре дорого и не есть хорошая практика
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_ ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_ ? GL_LINEAR : GL_NEAREST);

        // Использовать шейдер
        glUseProgram(shader_secondary_.id());
        // Привязать геометрию
        glBindVertexArray(geometry_secondary_.vao_id());
        // Нарисовать геометрию (квадрат на весь экран) используя текстуру из предыдущего прохода
        glUniform1i(shader_secondary_.uniforms().frame_texture, 0);
        glDrawElements(GL_TRIANGLES, geometry_secondary_.index_count(), GL_UNSIGNED_INT, nullptr);

        // Сброс текстур
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /**
     * Имя примера
     * @return Строка с именем
     */
    const char *Passes::name()
    {
        return "Render passes";
    }

    /**
     * Событие смены разрешения
     * Может быть вызвано как изменением размера она так и изменением масштабирования
     */
    void Passes::on_resolution_change()
    {
        // Остановить рендеринг
        render_ = false;

        // Выгрузить текущий буфер кадра
        frame_buffer_primary_.unload();

        // Создать новый первичный кадровый буфер
        utils::gl::FrameBufferAttachmentInfo color{GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0, GL_NEAREST};
        utils::gl::RenderBufferAttachmentInfo depth_stencil{GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT};
        frame_buffer_primary_ = utils::gl::FrameBuffer(
                (GLsizei)((float)g_screen_width * scales_[scale_index_]),
                (GLsizei)((float)g_screen_height * scales_[scale_index_]),
                {color},
                {depth_stencil});

        // Включить рендеринг
        render_ = frame_buffer_primary_.ready();

        // Обновить строчку с разрешением
        resolution_ = std::to_string(frame_buffer_primary_.width()) +
                "x" + std::to_string(frame_buffer_primary_.height());
    }
}