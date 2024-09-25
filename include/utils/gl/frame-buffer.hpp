#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "resource.hpp"

namespace utils::gl
{
    /**
     * Информация о цветовом вложении кадрового буфера
     * Для каждого вложения будет создана текстура
     */
    struct FrameBufferAttachmentInfo
    {
        // Внутренний формат (формат хранения)
        GLint internal_format = GL_RGBA;
        // Внешний формат (формат выборки/записи)
        GLenum format = GL_RGBA;
        // Индекс вложения (location у фрагментного шейдера)
        GLuint binding = GL_COLOR_ATTACHMENT0;
        // Тип желаемой фильтрации (GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR)
        GLint filtering = GL_NEAREST;
    };

    /**
     * Информация о рендер-вложении кадрового буфера
     * Для каждого такого вложения будет создан рендер-буфер
     * Рендер-буферы аналогичны текстурам, но не пригодны для выборки/записи в шейдере
     */
    struct RenderBufferAttachmentInfo
    {
        // Внутренний формат (формат хранения)
        GLint internal_format = GL_DEPTH32F_STENCIL8;
        // Тип вложения
        GLuint binding = GL_DEPTH_STENCIL_ATTACHMENT;
    };

    /**
     * Обертка над буфером кадра
     * Кадровый буфер содержит "цели" рендеринга (render targets) в которые происходит запись изображения/данных
     */
    class FrameBuffer final : public Resource
    {
    public:
        /**
         * Конструктор по умолчанию
         * Не создает ресурсов OpenGL, создает пустой объект
         */
        FrameBuffer()
            : Resource()
            , id_(0)
            , width_(0)
            , height_(0)
        {}

        /**
         * Основной конструктор (создает OpenGL ресурсы)
         * @param width Ширина вложений буфера
         * @param height Высота вложений буфера
         * @param tx_att_infos Описания для текстурных вложений
         * @param rb_att_infos Описания для вложений рендер-буфера
         */
        FrameBuffer(GLsizei width
                    , GLsizei height
                    , const std::vector<FrameBufferAttachmentInfo>& tx_att_infos
                    , const std::vector<RenderBufferAttachmentInfo>& rb_att_infos)
            : Resource()
            , id_(0)
            , width_(width)
            , height_(height)
        {
            // Добавление текстурных вложений
            for(const auto& info: tx_att_infos)
            {
                GLuint id;
                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, info.internal_format, width_, height_, 0, info.format, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filtering);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filtering);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);

                attachments_tx_.push_back(id);
                bindings_tx_.push_back(info.binding);
            }

            // Добавление вложение рендер-буфера
            for(const auto& info: rb_att_infos)
            {
                GLuint id;
                glGenRenderbuffers(1, &id);
                glBindRenderbuffer(GL_RENDERBUFFER, id);
                glRenderbufferStorage(GL_RENDERBUFFER, info.internal_format, width_, height_);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);

                attachments_rb_.push_back(id);
                bindings_rb_.push_back(info.binding);
            }

            // Дальнейшая инициализация не имеет смысла без вложений
            if(attachments_tx_.empty() && attachments_rb_.empty())
            {
                return;
            }

            // Генерация ресурса OpenGL (кадровый буфер)
            glGenFramebuffers(1, &id_);
            glBindFramebuffer(GL_FRAMEBUFFER, id_);

            // Добавить текстурные вложения (кол-во привязок и вложений совпадает)
            for (unsigned i = 0; i < attachments_tx_.size(); i++)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, bindings_tx_[i], GL_TEXTURE_2D, attachments_tx_[i], 0);
            }

            // Добавить вложения рендер-буфера (кол-во привязок и вложений совпадает)
            for (unsigned i = 0; i < attachments_rb_.size(); i++)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, bindings_rb_[i], GL_RENDERBUFFER, attachments_rb_[i]);
            }

            // Указать доступные для записи в текущем проходе вложения (по умолчанию - все текстурные вложения)
            if(!bindings_tx_.empty())
            {
                glDrawBuffers((GLsizei)bindings_tx_.size(), bindings_tx_.data());
            }

            // Завершение работы с кадровым буфером
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Готово к использованию
            loaded_ = true;
        }

        /**
         * Запрет копирования через конструктор (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         */
        FrameBuffer(const FrameBuffer& other) = delete;

        /**
         * Перемещение через конструктор (при конструировании из rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         */
        FrameBuffer(FrameBuffer&& other) noexcept
                : Resource(std::move(other))
                , id_(other.id_)
                , width_(other.width_)
                , height_(other.height_)
                , attachments_tx_(std::move(other.attachments_tx_))
                , attachments_rb_(std::move(other.attachments_rb_))
                , bindings_tx_(std::move(other.bindings_tx_))
                , bindings_rb_(std::move(other.bindings_rb_))
        {
            other.id_ = 0;
            other.width_ = 0;
            other.height_ = 0;
        }

        /**
         * Уничтожает OpenGL ресурсы
         */
        ~FrameBuffer() override
        {
            unload();
        }

        /**
         * Запрет копирования через присвоение (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        FrameBuffer& operator=(const FrameBuffer& other) = delete;

        /**
         * Перемещение через присвоение (при присвоении rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        FrameBuffer& operator=(FrameBuffer&& other) noexcept
        {
            if (&other == this) return *this;

            unload();

            std::swap(loaded_, other.loaded_);
            std::swap(id_, other.id_);
            std::swap(width_, other.width_);
            std::swap(height_, other.height_);
            std::swap(attachments_tx_, other.attachments_tx_);
            std::swap(attachments_rb_, other.attachments_rb_);
            std::swap(bindings_tx_, other.bindings_tx_);
            std::swap(bindings_rb_, other.bindings_rb_);

            return *this;
        }

        /**
         * Получить ID ресурса OpenGL
         * @return ID
         */
        [[nodiscard]] GLuint id() const
        {
            return id_;
        }

        /**
         * Получить список OpenGL дескрипторов текстурных вложений
         * @return Константная ссылка на список
         */
        [[nodiscard]] const std::vector<GLuint>& attachments_tx() const
        {
            return attachments_tx_;
        }

        /**
         * Получить список OpenGL дескрипторов вложений рендер-буфера
         * @return Константная ссылка на список
         */
        [[nodiscard]] const std::vector<GLuint>& attachments_rb() const
        {
            return attachments_rb_;
        }

        /**
         * Получить ширину буфера
         * @return Ширина
         */
        [[nodiscard]] GLsizei width() const
        {
            return width_;
        }

        /**
         * Получить ширину буфера
         * @return Ширина
         */
        [[nodiscard]] GLsizei height() const
        {
            return height_;
        }

        /**
         * Выгрузка ресурса
         */
        void unload() override
        {
            if(!attachments_tx_.empty()) glDeleteTextures((GLsizei)attachments_tx_.size(), attachments_tx_.data());
            if(!attachments_rb_.empty()) glDeleteRenderbuffers((GLsizei)attachments_rb_.size(), attachments_rb_.data());
            if(id_) glDeleteFramebuffers(1, &id_);

            id_ = 0;
            attachments_tx_.clear();
            attachments_rb_.clear();
            bindings_tx_.clear();
            bindings_rb_.clear();
            loaded_ = false;
        }

    private:
        GLuint id_;                              // OpenGL дескриптор кадрового буфера
        GLsizei width_;                          // Ширина
        GLsizei height_;                         // Высота
        std::vector<GLuint> attachments_tx_;     // OpenGL дескрипторы вложений (текстур)
        std::vector<GLuint> attachments_rb_;     // OpenGL дескрипторы вложений (рендер-буферов)
        std::vector<GLuint> bindings_tx_;        // Привязки вложений текстур (location у фрагментного шейдера)
        std::vector<GLuint> bindings_rb_;        // Привязки вложений рендер-буфера
    };
}