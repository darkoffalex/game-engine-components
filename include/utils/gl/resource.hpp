#pragma once

#include <glad/glad.h>

namespace utils::gl
{
    /**
     * Интерфейс ресурса OpenGL
     */
    class Resource
    {
    public:
        /**
         * Конструктор по умолчанию
         */
        Resource():loaded_(false){}

        /**
         * Запрет копирования через конструктор (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         */
        Resource(const Resource& other) = delete;

        /**
         * Перемещение через конструктор (при конструировании из rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         */
        Resource(Resource&& other) noexcept
            :loaded_(other.loaded_)
        {
            other.loaded_ = false;
        }

        /**
         * Деструктор по умолчанию
         */
        virtual ~Resource() = default;

        /**
         * Запрет копирования через присвоение (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Resource& operator=(const Resource& other) = delete;

        /**
         * Перемещение через присвоение (при присвоении rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Resource& operator=(Resource&& other) noexcept
        {
            if (&other == this) return *this;

            loaded_ = false;

            std::swap(loaded_, other.loaded_);

            return *this;
        }

        /**
         * Выгрузка ресурса
         */
        virtual void unload() = 0;

        /**
         * Готовность к использованию
         * @return Статус
         */
        [[nodiscard]] bool ready() const
        {
            return loaded_;
        }

    protected:
        bool loaded_;
    };
}