#pragma once

#include <glad/glad.h>

namespace utils::gl
{

    /**
     * Описание атрибута вершины для шейдера
     */
    struct VertexAttributeInfo
    {
        // Номер положения (location у шейдера)
        GLuint location = 0;
        // Кол-во компонентов (например, для vec3 будет 3 компонента)
        GLint componnent_count = 0;
        // Тип компонентов (float, int, прочие)
        GLenum component_type = GL_FLOAT;
        // Автоматически нормализовать перед подачей в шейдер
        GLboolean normalize = GL_FALSE;
        // Сдвиг для атрибута в одной структуре вершины
        GLsizeiptr offset = 0;
    };

    /**
     * Обертка над буфером геометрии (вершины, индексы)
     * @tparam V Структура вершины
     */
    template <class V>
    class Geometry final
    {
    public:
        /**
         * Основной конструктор (создает OpenGL ресурсы)
         * @param vertices Массив вершин
         * @param indices Массив индексов
         * @param attributes Список описаний атрибутов вершины для шейдера
         */
        explicit Geometry(const std::vector<V>& vertices, const std::vector<GLuint>& indices, const std::vector<VertexAttributeInfo>& attributes)
            : vbo_id_(0)
            , ebo_id_(0)
            , vao_id_(0)
            , vertext_count_(0)
            , index_count_(0)
        {
            // Информация для дальнейшего использования (например, при отрисовке)
            vertext_count_ = static_cast<GLsizei>(vertices.size());
            index_count_ = static_cast<GLsizei>(vertices.size());

            // Убелиться в корректности данных
            assert(vertext_count_ > 0);
            assert(index_count_ > 0);

            // Генерация необходимых ресурсов OpenGL (vertex buffer, element buffer, vertex array object)
            glGenBuffers(1, &vbo_id_);
            glGenBuffers(1, &ebo_id_);
            glGenVertexArrays(1, &vao_id_);

            // Работа с VAO
            glBindVertexArray(vao_id_);

            // Приавзка буффера вершин
            glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(V) * vertext_count_), vertices.data(), GL_STATIC_DRAW);

            // Привязка буфера индексов
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id_);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(GLuint) * index_count_), indices.data(), GL_STATIC_DRAW);

            // Пояснения шейдеру как понимать данные из активного VBO (буфера вершин) в контексте активного VAO
            setup_vertex_attributes(attributes);

            // Завершение работы с VAO
            glBindVertexArray(0);
        }

        /**
         * Запрет копирования через конструктор (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         */
        Geometry(const Geometry& other) = delete;

        /**
         * Перемещение через конструктор (при конструировании из rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         */
        Geometry(Geometry&& other) noexcept
            : vbo_id_(0)
            , ebo_id_(0)
            , vao_id_(0)
            , vertext_count_(0)
            , index_count_(0)
        {
            other.vao_id_ = 0;
            other.ebo_id_ = 0;
            other.vao_id_ = 0;
            other.vertext_count_ = 0;
            other.index_count_ = 0;
        }

        /**
         * Уничтожает OpenGL ресурсы
         */
        ~Geometry()
        {
            if (vbo_id_) glDeleteBuffers(1, &vbo_id_);
            if (ebo_id_) glDeleteBuffers(1, &ebo_id_);
            if (vao_id_) glDeleteVertexArrays(1, &vao_id_);
            vbo_id_ = 0;
            ebo_id_ = 0;
            vao_id_ = 0;
            vertext_count_ = 0;
            index_count_ = 0;
        }

        /**
         * Запрет копирования через присвоение (нет смысла копировать многократно используемый ресурс)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Geometry& operator=(const Geometry& other) = delete;

        /**
         * Перемещение через присвоение (при присвоении rvalue-значения возможен обмен ресурса)
         * @param other Другой объект
         * @return Ссылка на текущий объект
         */
        Geometry& operator=(Geometry&& other) noexcept
        {
            if (&other == this) return *this;

            if (vbo_id_) glDeleteBuffers(1, &vbo_id_);
            if (ebo_id_) glDeleteBuffers(1, &ebo_id_);
            if (vao_id_) glDeleteVertexArrays(1, &vao_id_);
            vbo_id_ = 0;
            ebo_id_ = 0;
            vao_id_ = 0;
            vertext_count_ = 0;
            index_count_ = 0;

            std::swap(vbo_id_, other.vbo_id_);
            std::swap(ebo_id_, other.ebo_id_);
            std::swap(vao_id_, other.vao_id_);
            std::swap(vertext_count_, other.vertext_count_);
            std::swap(index_count_, other.index_count_);

            return *this;
        }

        /**
         * Получить OpenGL дескриптор vertex buffer object
         * @return Дескриптор ресурса
         */
        [[nodiscard]] GLuint vbo_id() const
        {
            return vbo_id_;
        }

        /**
         * Получить OpenGL дескриптор element buffer object
         * @return Дескриптор ресурса
         */
        [[nodiscard]] GLuint ebo_id() const
        {
            return ebo_id_;
        }

        /**
         * Получить OpenGL дескриптор vertex array object
         * @return Дескриптор ресурса
         */
        [[nodiscard]] GLuint vao_id() const
        {
            return vao_id_;
        }

        /**
         * Получить кол-во вершин
         * @return Кол-во
         */
        [[nodiscard]] GLsizei vertex_count() const
        {
            return vertext_count_;
        }

        /**
         * Получить кол-во индексов
         * @return Кол-во
         */
        [[nodiscard]] GLsizei index_count() const
        {
            return index_count_;
        }

    protected:
        /**
         * Пояснить как соотносить данные вершины и атрибуты в шейдере
         * @param attributes Массив с информацией об атрибутах вершины
         */
        void setup_vertex_attributes(const std::vector<VertexAttributeInfo>& attributes) const
        {
            for(const auto&[location, componnent_count, component_type, normalize, offset] : attributes)
            {
                glVertexAttribPointer(location
                    , componnent_count
                    , component_type
                    , normalize
                    , sizeof(V)
                    , reinterpret_cast<const void*>(offset)
                    );

                glEnableVertexArrayAttrib(vao_id_, location);
            }
        }

    private:
        GLuint vbo_id_;             // OpenGL дескриптор вершинного буфера
        GLuint ebo_id_;             // OpenGL дескриптор индексного буфера
        GLuint vao_id_;             // OpenGL дескриптор VAO объекта
        GLsizei vertext_count_;     // Кол-во вершин
        GLsizei index_count_;       // Кол-во индексов
    };
}