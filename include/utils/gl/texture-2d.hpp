#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.hpp"

namespace utils::gl
{
    /**
     * Обертка над текстурой OpenGL
     */
    class Texture2D final : public Resource
    {
    public:
        /**
         * Типы цветового пространства текстуры
         */
        enum class EColorSpace: unsigned
        {
            GRAYSCALE = 0,          // Оттенки серого цвета
            GRAYSCALE_ALPHA,        // Оттенки серого + альфа канал
            RGB,                    // Цветное изображение
            RGB_ALPHA,              // Цветное изображение + альфа канал
            SRGB,                   // Цветное изображение в sRGB пространстве
            SRGB_ALPHA              // Цветное изображение в sRGB пространстве + альфа канал
        };

        /**
         * Конструктор по умолчанию
         * Не создает ресурсов OpenGL, создает пустой объект
         */
        Texture2D()
                : Resource()
                , id_(0)
                , width_(0)
                , height_(0)
                , mip_(false)
        {}

        /**
         * Основной конструктор (создает OpenGL ресурсы)
         * @param data Указатель на данные
         * @param width Ширина текстуры
         * @param height Высота текстуры
         * @param desired_filtration Желаемая фильтрация (GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR)
         * @param color_space Цветовое пространство
         * @param mip Генерация мип-уровней
         * @param data_type Тип данных цветового канала
         */
        Texture2D(void* data,
                  GLsizei width,
                  GLsizei height,
                  GLint desired_filtration,
                  EColorSpace color_space,
                  bool mip,
                  GLuint data_type = GL_UNSIGNED_BYTE)
                : Resource()
                , id_(0)
                , width_(width)
                , height_(height)
                , mip_(mip)
        {
            // Данные должны быть предоставлены
            assert(data != nullptr);

            // Генерация идентификатора текстуры
            glGenTextures(1, &id_);
            // Привязываемся к текстуре по идентификатору (работаем с текстурой)
            glBindTexture(GL_TEXTURE_2D, this->id_);

            // Три-линейная фильтрация возможна только если включены мип-уровни
            // В противном случае используется обычная линейная фильтрация
            if(!mip_ && desired_filtration == GL_LINEAR_MIPMAP_LINEAR)
            {
                desired_filtration = GL_LINEAR;
            }

            // Фильтр при уменьшении (когда один пиксель вмещает несколько текселей текстуры)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desired_filtration);
            // Когда на один тексель текстуры приходится несколько пикселей (объекты рассматриваются вблизи)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desired_filtration);

            // Соответствия для цветовых пространств и форматов хранения (внутренний, данные) и чтения (в шейдере)
            std::unordered_map<EColorSpace, std::pair<GLint, GLint>> color_space_formats {
                    {EColorSpace::GRAYSCALE, {GL_RED, GL_RED}},
                    {EColorSpace::GRAYSCALE_ALPHA, {GL_RG, GL_RG}},
                    {EColorSpace::RGB, {GL_RGB, GL_RGB}},
                    {EColorSpace::RGB_ALPHA, {GL_RGBA, GL_RGBA}},
                    {EColorSpace::SRGB, {GL_SRGB, GL_RGB}},
                    {EColorSpace::SRGB_ALPHA, {GL_SRGB_ALPHA, GL_RGBA}},
            };

            // Устанавливаем данные текстуры (загрузка в текстурную память)
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         color_space_formats[color_space].first,
                         width_,
                         height_,
                         0,
                         color_space_formats[color_space].second,
                         data_type,
                         data);

            // Генерация мип-уровней (если нужно)
            if (this->mip_) glGenerateMipmap(GL_TEXTURE_2D);

            // Прекращаем работать с текстурой
            glBindTexture(GL_TEXTURE_2D, 0);

            // Готово к использованию
            loaded_ = true;
        }

        /**
         * Запрет копирования через конструктор (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         */
        Texture2D(const Texture2D& other) = delete;

        /**
         * Перемещение через конструктор (при конструировании из rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         */
        Texture2D(Texture2D&& other) noexcept
                : Resource(std::move(other))
                , id_(other.id_)
                , width_(other.width_)
                , height_(other.height_)
                , mip_(other.mip_)
        {
            other.id_ = 0;
            other.width_ = 0;
            other.height_ = 0;
            other.mip_ = false;
        }

        /**
         * Уничтожает OpenGL ресурсы
         */
        ~Texture2D() override
        {
            unload();
        }

        /**
         * Запрет копирования через присвоение (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Texture2D& operator=(const Texture2D& other) = delete;

        /**
         * Перемещение через присвоение (при присвоении rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Texture2D& operator=(Texture2D&& other) noexcept
        {
            if (&other == this) return *this;

            if (id_) glDeleteTextures(1, &id_);
            loaded_ = false;
            id_ = 0;
            width_ = 0;
            height_ = 0;
            mip_ = false;

            std::swap(loaded_, other.loaded_);
            std::swap(id_, other.id_);
            std::swap(width_, other.width_);
            std::swap(height_, other.height_);
            std::swap(mip_, other.mip_);

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
         * Получить ширину текстуры
         * @return Ширина
         */
        [[nodiscard]] GLuint width() const
        {
            return width_;
        }

        /**
         * Получить ширину текстуры
         * @return Ширина
         */
        [[nodiscard]] GLuint height() const
        {
            return height_;
        }

        /**
         * Выгрузка ресурса
         */
        void unload() override
        {
            if (id_) glDeleteTextures(1, &id_);

            id_ = 0;
            width_ = 0;
            height_ = 0;
            mip_ = false;
            loaded_ = false;
        }

    private:
        GLuint id_;             // OpenGL дескриптор текстуры
        GLsizei width_;         // Ширина текстуры
        GLsizei height_;        // Высота текстуры
        bool mip_;              // Генерация мип-уровней
    };
}