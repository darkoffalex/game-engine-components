#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>

namespace utils::gl
{
    /**
     * Обертка над шейдерной программой
     * @tparam L Структура с идентификаторами uniform-переменных
     * @tparam T Тип полей вышеупомянутой структуры
     */
    template <class L, typename T>
    class Shader final
    {
    public:
        /**
         * Конструктор по умолчанию
         * Не создает ресурс OpenGL, создает пустой объект
         */
        Shader(): initialized_(false), id_(0), locations_({})
        {}

        /**
         * Основной конструктор
         * @param sources Ассоциативный массив исходников (тип - исходник)
         * @param uniforms Наименования uniform переменных в шейдере (с учетом порядка и кол-ва в структуре L)
         */
        Shader(const std::unordered_map<GLuint, std::string>& sources, const std::vector<std::string>& uniforms)
            : initialized_(false)
            , id_(0)
            , locations_({})
        {
            // Создать программу
            id_ = glCreateProgram();

            // Создать и скомпилировать шейдеры из исходных текстов
            std::vector<GLuint> shader_ids;
            for(auto& [type, source] : sources)
            {
                GLuint sid = compile_shader_source(type, source);
                glAttachShader(this->id_, sid);
                shader_ids.push_back(sid);
            }

            // Собрать программу
            glLinkProgram(this->id_);

            // Удалить сущности шейдеров (программа собрана)
            for (const auto& sid : shader_ids)
            {
                glDeleteShader(sid);
            }

            // Проверка сборки программы
            GLint success;
            glGetProgramiv(this->id_, GL_LINK_STATUS, &success);
            if (!success)
            {
                std::string msg;
                GLint msg_len = 0;

                glGetProgramiv(this->id_, GL_INFO_LOG_LENGTH, &msg_len);
                msg.resize(msg_len);
                glGetProgramInfoLog(this->id_, msg_len, nullptr, msg.data());

                throw std::runtime_error("[GL] shader linking error: " + msg);
            }

            // Получить ID'ы uniform-переменных
            if(!uniforms.empty()) init_uniform_locations(uniforms);

            // Готово к использованию
            initialized_ = true;
        }

        /**
         * Запрет копирования через конструктор (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         */
        Shader(const Shader& other) = delete;

        /**
         * Перемещение через конструктор (при конструировании из rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         */
        Shader(Shader&& other) noexcept
            : initialized_(other.initialized_)
            , id_(other.id_)
            , locations_(other.locations_)
        {
            other.initialized_ = false;
            other.id_ = 0;
            other.locations_ = {};
        }

        /**
         * Уничтожает OpenGL ресурс
         */
        ~Shader()
        {
            if (id_) glDeleteProgram(id_);

            initialized_ = false;
            id_ = 0;
            locations_ = {};
        }

        /**
         * Запрет копирования через присвоение (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Shader& operator=(const Shader& other) = delete;

        /**
         * Перемещение через присвоение (при присвоении rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Shader& operator=(Shader&& other) noexcept
        {
            if (&other == this) return *this;

            if (id_) glDeleteProgram(id_);

            initialized_ = false;
            id_ = 0;
            locations_ = {};

            std::swap(this->initialized_, other.initialized_);
            std::swap(this->id_, other.id_);
            std::swap(this->locations_, other.locations_);

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
         * Получить структуру с идентификаторами uniform-переменных
         * @return Идентификаторы локаций
         */
        [[nodiscard]] const L& uniform_locations() const
        {
            return locations_;
        }

        /**
         * Получить готовность к использованию
         * @return Статус инициализации
         */
        [[nodiscard]] bool initialized() const
        {
            return initialized_;
        }

    protected:
        /**
         * Получить указатель на поле в структуре uniform-переменных
         * @param index Индекс поля
         * @return Указатель
         */
        T* get_location_ptr(size_t index)
        {
            assert(sizeof(T)* index <= sizeof(L));
            auto p = reinterpret_cast<T*>(&locations_);
            return p + index;
        }

        /**
         * Установить значения (идентификаторы) uniform-переменных в структуре
         * @param uniforms Названия uniform-переменных в шейдере
         */
        void init_uniform_locations(const std::vector<std::string>& uniforms)
        {
            for(size_t i = 0; i < uniforms.size(); i++)
            {
                auto* ptr = get_location_ptr(i);
                auto& name = uniforms[i];

                *ptr = glGetUniformLocation(id_, name.c_str());
            }
        }

        /**
         * Компиляция исходников шейдера
         * @param type Тип шейдера (константы OpenGL - GL_VERTEX_SHADER, GL_FRAGMENT_SHADER и др, смм. документацию)
         * @param source Исходный текст шейдера
         * @return Идентификатор созданного шейдера
         */
        static GLuint compile_shader_source(const GLuint type, const std::string& source)
        {
            const GLuint id = glCreateShader(type);
            const GLchar* source_ptr = source.c_str();
            glShaderSource(id, 1, &source_ptr, nullptr);
            glCompileShader(id);

            GLint success;
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                std::string msg;
                GLint msg_len = 0;

                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &msg_len);
                msg.resize(msg_len);
                glGetShaderInfoLog(id, msg_len, nullptr, msg.data());

                std::string full_msg = "[GL] shader compile error ";
                full_msg += "(type - ";
                full_msg += std::to_string(type);
                full_msg += "): " + msg;
                throw std::runtime_error(full_msg);
            }

            return id;
        }

    private:
        bool initialized_;  // Готовность к использованию
        GLuint id_;         // OpenGL дескриптор шейдерной программы
        L locations_;       // Структура идентификаторов uniform-переменных
    };
}